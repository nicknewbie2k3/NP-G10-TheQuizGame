#include "game_server.h"
#include <random>
#include <algorithm>
#include <sstream>
#include <ctime>
#include <iostream>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

std::string generateGamePin() {
    static const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);
    
    std::string pin;
    for (int i = 0; i < 6; ++i) {
        pin += charset[dis(gen)];
    }
    return pin;
}

void sendToClient(struct lws* wsi, const std::string& message) {
    if (!wsi) return;
    
    size_t len = message.length();
    unsigned char* buf = new unsigned char[LWS_PRE + len];
    memcpy(&buf[LWS_PRE], message.c_str(), len);
    
    lws_write(wsi, &buf[LWS_PRE], len, LWS_WRITE_TEXT);
    delete[] buf;
}

void broadcastToGame(const std::string& gamePin, const std::string& message, struct lws* excludeWsi, ServerContext* ctx) {
    auto it = ctx->games.find(gamePin);
    if (it == ctx->games.end()) return;
    
    auto game = it->second;
    
    // Send to all players
    for (const auto& player : game->players) {
        if (player->wsi && player->wsi != excludeWsi) {
            sendToClient(player->wsi, message);
        }
    }
    
    // Send to host
    if (game->hostWsi && game->hostWsi != excludeWsi) {
        sendToClient(game->hostWsi, message);
    }
}

void broadcastToActivePlayers(const std::string& gamePin, const std::string& message, struct lws* excludeWsi, ServerContext* ctx) {
    auto it = ctx->games.find(gamePin);
    if (it == ctx->games.end()) return;
    
    auto game = it->second;
    
    // Send only to non-eliminated players
    for (const auto& player : game->players) {
        if (player->wsi && player->wsi != excludeWsi && !player->isEliminated) {
            sendToClient(player->wsi, message);
        }
    }
    
    // Send to host
    if (game->hostWsi && game->hostWsi != excludeWsi) {
        sendToClient(game->hostWsi, message);
    }
}

std::shared_ptr<Player> findPlayerByWsi(std::shared_ptr<Game> game, struct lws* wsi) {
    for (const auto& player : game->players) {
        if (player->wsi == wsi) {
            return player;
        }
    }
    return nullptr;
}

std::shared_ptr<Player> findPlayerById(std::shared_ptr<Game> game, const std::string& playerId) {
    for (const auto& player : game->players) {
        if (player->id == playerId) {
            return player;
        }
    }
    return nullptr;
}

std::shared_ptr<Game> findGameByWsi(struct lws* wsi, ServerContext* ctx) {
    auto it = ctx->wsToGamePin.find(wsi);
    if (it != ctx->wsToGamePin.end()) {
        auto gameIt = ctx->games.find(it->second);
        if (gameIt != ctx->games.end()) {
            return gameIt->second;
        }
    }
    return nullptr;
}

void handleCreateGame(struct lws* wsi, ServerContext* ctx) {
    std::string gamePin = generateGamePin();
    
    auto game = std::make_shared<Game>();
    game->pin = gamePin;
    game->hostWsi = wsi;
    game->gameState = "lobby";
    game->currentRound = 1;
    game->currentQuestion = 0;
    game->questionsPerRound = 2;
    game->currentPlayerTurn = 0;
    game->isSpeedPhase = false;
    game->waitingForHost = false;
    game->currentTurnNumber = 1;
    game->turnTimeRemaining = 45;
    
    // Copy questions from server context
    game->questions = ctx->mockQuestions;
    game->questionPacks = ctx->questionPacks;
    
    ctx->games[gamePin] = game;
    ctx->wsToGamePin[wsi] = gamePin;
    
    json response;
    response["type"] = "game_created";
    response["gamePin"] = gamePin;
    
    sendToClient(wsi, response.dump());
    
    std::cout << "🎮 Game created with PIN: " << gamePin << std::endl;
}

void handleJoinGame(struct lws* wsi, const std::string& gamePin, const std::string& playerName, ServerContext* ctx) {
    auto it = ctx->games.find(gamePin);
    if (it == ctx->games.end()) {
        json response;
        response["type"] = "error";
        response["message"] = "Game not found";
        sendToClient(wsi, response.dump());
        return;
    }
    
    auto game = it->second;
    
    // Check if name is taken
    for (const auto& p : game->players) {
        if (p->name == playerName) {
            json response;
            response["type"] = "error";
            response["message"] = "Player name already taken";
            sendToClient(wsi, response.dump());
            return;
        }
    }
    
    // Create player
    auto player = std::make_shared<Player>();
    player->id = generateGamePin(); // Use same function for unique ID
    player->name = playerName;
    player->wsi = wsi;
    player->connected = true;
    player->hasAnswered = false;
    player->isEliminated = false;
    player->score = 0;
    player->roundScore = 0;
    
    game->players.push_back(player);
    ctx->wsToGamePin[wsi] = gamePin;
    ctx->wsToPlayerId[wsi] = player->id;
    
    // Send join confirmation to player
    json joinResponse;
    joinResponse["type"] = "join_success";
    joinResponse["playerId"] = player->id;
    joinResponse["playerName"] = playerName;
    joinResponse["gamePin"] = gamePin;
    sendToClient(wsi, joinResponse.dump());
    
    // Broadcast player list update
    json playerList = json::array();
    for (const auto& p : game->players) {
        json pObj;
        pObj["id"] = p->id;
        pObj["name"] = p->name;
        pObj["connected"] = p->connected;
        playerList.push_back(pObj);
    }
    
    json broadcast;
    broadcast["type"] = "player_joined";
    broadcast["playerName"] = playerName;
    broadcast["players"] = playerList;
    
    broadcastToGame(gamePin, broadcast.dump(), nullptr, ctx);
    
    std::cout << "👤 Player '" << playerName << "' joined game " << gamePin << std::endl;
}

void handleStartGame(struct lws* wsi, ServerContext* ctx) {
    auto game = findGameByWsi(wsi, ctx);
    if (!game) return;
    
    if (game->hostWsi != wsi) {
        json response;
        response["type"] = "error";
        response["message"] = "Only host can start the game";
        sendToClient(wsi, response.dump());
        return;
    }
    
    if (game->players.size() < 2) {
        json response;
        response["type"] = "error";
        response["message"] = "Need at least 2 players to start";
        sendToClient(wsi, response.dump());
        return;
    }
    
    game->gameState = "round1";
    game->currentRound = 1;
    game->currentQuestion = 0;
    
    // Initialize active players
    game->activePlayers = game->players;
    
    // Shuffle questions
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(game->questions.begin(), game->questions.end(), g);
    
    // Send game started notification
    json broadcast;
    broadcast["type"] = "game_started";
    broadcast["round"] = 1;
    broadcast["totalRounds"] = 2;
    
    broadcastToGame(game->pin, broadcast.dump(), nullptr, ctx);
    
    // Send first question
    if (!game->questions.empty()) {
        const auto& q = game->questions[0];
        
        json questionMsg;
        questionMsg["type"] = "new_question";
        questionMsg["question"]["id"] = q.id;
        questionMsg["question"]["text"] = q.text;
        questionMsg["question"]["options"] = q.options;
        questionMsg["question"]["timeLimit"] = q.timeLimit;
        questionMsg["questionNumber"] = 1;
        questionMsg["totalQuestions"] = game->questionsPerRound;
        questionMsg["round"] = game->currentRound;
        
        broadcastToGame(game->pin, questionMsg.dump(), nullptr, ctx);
    }
    
    std::cout << "🚀 Game " << game->pin << " started with " << game->players.size() << " players" << std::endl;
}

void handleSubmitAnswer(struct lws* wsi, int questionId, int answer, ServerContext* ctx) {
    auto game = findGameByWsi(wsi, ctx);
    if (!game) return;
    
    auto player = findPlayerByWsi(game, wsi);
    if (!player || player->hasAnswered) return;
    
    player->hasAnswered = true;
    player->answerTime = std::chrono::steady_clock::now();
    
    // Check if answer is correct
    const auto& question = game->questions[game->currentQuestion];
    bool isCorrect = (answer == question.correctAnswer);
    
    if (isCorrect) {
        player->roundScore += 100; // Base points
        player->score += 100;
    }
    
    game->answers[player->id] = answer;
    
    // Send answer received confirmation
    json response;
    response["type"] = "answer_received";
    response["correct"] = isCorrect;
    sendToClient(wsi, response.dump());
    
    // Check if all active players have answered
    bool allAnswered = true;
    for (const auto& p : game->activePlayers) {
        if (!p->isEliminated && !p->hasAnswered) {
            allAnswered = false;
            break;
        }
    }
    
    if (allAnswered) {
        // Show results
        json results;
        results["type"] = "question_results";
        results["correctAnswer"] = question.correctAnswer;
        
        json scores = json::object();
        for (const auto& p : game->players) {
            scores[p->id] = p->roundScore;
        }
        results["scores"] = scores;
        
        broadcastToGame(game->pin, results.dump(), nullptr, ctx);
    }
}

void handleNextQuestion(struct lws* wsi, ServerContext* ctx) {
    auto game = findGameByWsi(wsi, ctx);
    if (!game) return;
    
    if (game->hostWsi != wsi) return;
    
    // Reset answered status
    for (auto& p : game->players) {
        p->hasAnswered = false;
    }
    game->answers.clear();
    
    game->currentQuestion++;
    
    // Check if round is over
    if (game->currentQuestion >= game->questionsPerRound) {
        // Round over - eliminate lowest scorer
        if (game->currentRound == 1 && game->activePlayers.size() > 2) {
            // Find lowest scorer
            std::shared_ptr<Player> lowestPlayer = nullptr;
            int lowestScore = INT_MAX;
            std::vector<std::shared_ptr<Player>> lowestPlayers;
            
            for (const auto& p : game->activePlayers) {
                // if (!p->isEliminated && p->roundScore < lowestScore) {
                //     lowestScore = p->roundScore;
                //     lowestPlayer = p;
                // }

                if(!p->isEliminated) {
                    if (p->roundScore < lowestScore) {
                        lowestScore = p->roundScore;
                        // reset lowestPlayers then add this player (p)
                        lowestPlayers.clear();
                        lowestPlayers.push_back(p);
                    } else if (p->roundScore == lowestScore) {
                        // add this player (p) to the list of lowest players
                        lowestPlayers.push_back(p);
                    }
                }
            }

            if(lowestPlayers.size() > 1){
                std::cout << "⚠️ Tie detected! " << lowestPlayers.size() << " players have score " << lowestScore << std::endl;

                // Set up tiebreaker round
                game->isTieBreaker = true;
                game->tieBreakerIds.clear();
                game->speedResponses.clear();
                
                // Add tied players to tiebreaker list
                for (const auto& p : lowestPlayers) {
                    game->tieBreakerIds.push_back(p->id);
                }
                
                game->turnStartTime = std::chrono::steady_clock::now();
                
                // Notify players of tiebreaker round
                json tiebreakStart;
                tiebreakStart["type"] = "tiebreak_start";
                tiebreakStart["message"] = "Tie Detected! Speed Question Tiebreaker";
                tiebreakStart["tiedPlayerCount"] = lowestPlayers.size();
                broadcastToGame(game->pin, tiebreakStart.dump(), nullptr, ctx);
                
                // Send speed question to all players
                if (!ctx->speedQuestions.empty()) {
                    const auto& sq = ctx->speedQuestions[0];
                    json speedQ;
                    speedQ["type"] = "tiebreak_question";
                    speedQ["question"]["id"] = sq.id;
                    speedQ["question"]["text"] = sq.text;
                    broadcastToGame(game->pin, speedQ.dump(), nullptr, ctx);
                }
                return; // Exit early - wait for tiebreak answers
            }else{
                auto lowestPlayer = lowestPlayers[0];
                lowestPlayer->isEliminated = true;
                game->eliminatedPlayers.push_back(lowestPlayer);
                game->activePlayers.erase(
                    std::remove(game->activePlayers.begin(), game->activePlayers.end(), lowestPlayer),
                    game->activePlayers.end()
                );
                
                json elimination;
                elimination["type"] = "player_eliminated";
                elimination["playerId"] = lowestPlayer->id;
                elimination["playerName"] = lowestPlayer->name;
                elimination["reason"] = "Lowest score in Round 1";
                
                broadcastToGame(game->pin, elimination.dump(), nullptr, ctx);
            }
            
            // if (lowestPlayer) {
            //     lowestPlayer->isEliminated = true;
            //     game->eliminatedPlayers.push_back(lowestPlayer);
            //     game->activePlayers.erase(
            //         std::remove(game->activePlayers.begin(), game->activePlayers.end(), lowestPlayer),
            //         game->activePlayers.end()
            //     );
                
            //     json elimination;
            //     elimination["type"] = "player_eliminated";
            //     elimination["playerId"] = lowestPlayer->id;
            //     elimination["playerName"] = lowestPlayer->name;
            //     elimination["reason"] = "Lowest score in Round 1";
                
            //     broadcastToGame(game->pin, elimination.dump(), nullptr, ctx);
            // }
        }
        
        json roundEnd;
        roundEnd["type"] = "round_complete";
        roundEnd["round"] = game->currentRound;
        
        broadcastToGame(game->pin, roundEnd.dump(), nullptr, ctx);
        return;
    }
    
    // Send next question
    const auto& q = game->questions[game->currentQuestion];
    json questionMsg;
    questionMsg["type"] = "new_question";
    questionMsg["question"]["id"] = q.id;
    questionMsg["question"]["text"] = q.text;
    questionMsg["question"]["options"] = q.options;
    questionMsg["question"]["timeLimit"] = q.timeLimit;
    questionMsg["questionNumber"] = game->currentQuestion + 1;
    questionMsg["totalQuestions"] = game->questionsPerRound;
    questionMsg["round"] = game->currentRound;
    
    broadcastToGame(game->pin, questionMsg.dump(), nullptr, ctx);
}

void handleNextRound(struct lws* wsi, ServerContext* ctx) {
    auto game = findGameByWsi(wsi, ctx);
    if (!game) return;

    if (game->hostWsi != wsi) return;
    
    game->currentRound++;
    game->currentQuestion = 0;
    game->isSpeedPhase = true;
    
    // Reset round scores
    for (auto& p : game->players) {
        p->roundScore = 0;
    }
    
    if (game->currentRound == 2) {
        // Start Round 2 - Speed Question Phase (for determining turn order)
        game->isSpeedOrderPhase = true;  // Mark this as speed order phase - no elimination
        
        json round2Start;
        round2Start["type"] = "round2_start";
        round2Start["message"] = "Round 2: Turn-Based Questions";
        round2Start["phase"] = "speed";
        
        broadcastToGame(game->pin, round2Start.dump(), nullptr, ctx);
        
        // Send speed question
        if (!ctx->speedQuestions.empty()) {
            const auto& sq = ctx->speedQuestions[0];
            json speedQ;
            speedQ["type"] = "speed_question";
            speedQ["question"]["id"] = sq.id;
            speedQ["question"]["text"] = sq.text;
            
            broadcastToGame(game->pin, speedQ.dump(), nullptr, ctx);
        }
    }
}

void handleSpeedAnswer(struct lws* wsi, const std::string& questionId, const std::string& answer, ServerContext* ctx) {
    auto game = findGameByWsi(wsi, ctx);
    if (!game) return;
    
    auto player = findPlayerByWsi(game, wsi);
    if (!player) return;
    
    // Stop eliminated players from answering
    if (player->isEliminated) {
        json response;
        response["type"] = "error";
        response["message"] = "You have been eliminated";
        sendToClient(wsi, response.dump());
        return;
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - game->turnStartTime).count();
    
    game->speedResponses[player->id] = {answer, elapsed};
    
    json response;
    response["type"] = "speed_answer_received";
    sendToClient(wsi, response.dump());
    
    std::cout << "📝 Speed answer from " << player->name << ": " << answer << " (" << elapsed << "ms)" << std::endl;
    
    // Check if all active players have answered
    size_t activePlayerCount = 0;
    for (const auto& p : game->players) {
        if (!p->isEliminated) {
            activePlayerCount++;
        }
    }
    
    if (game->speedResponses.size() >= activePlayerCount) {
        // All players have answered - send results
        std::cout << "✅ All players answered speed question, sending results..." << std::endl;
        
        // Get the correct answer (assuming first speed question for now)
        std::string correctAnswer = "";
        if (!ctx->speedQuestions.empty()) {
            correctAnswer = ctx->speedQuestions[0].correctAnswer;
        }
        
        // Build results array sorted by response time
        std::vector<std::pair<std::string, std::pair<std::string, long>>> sortedResponses(
            game->speedResponses.begin(), 
            game->speedResponses.end()
        );
        
        std::sort(sortedResponses.begin(), sortedResponses.end(),
            [](const auto& a, const auto& b) {
                return a.second.second < b.second.second; // Sort by response time
            });
        
        json results;
        results["type"] = "speed_results";
        results["results"] = json::array();
        
        // std::string slowestPlayerId;
        // std::string slowestPlayerName;

        // List of incorrect players, fastest first
        std::vector<std::string> incorrectPlayers;
        std::string eliminatedId;
        std::string eliminatedName;
        
        for (const auto& [playerId, respPair] : sortedResponses) {
            auto p = findPlayerById(game, playerId);
            if (!p) continue;
            
            // Skip eliminated players from results
            if (p->isEliminated) continue;
            
            const std::string& playerAnswer = respPair.first;
            long responseTime = respPair.second;
            
            // Check if answer is correct (case-insensitive)
            std::string lowerAnswer = playerAnswer;
            std::transform(lowerAnswer.begin(), lowerAnswer.end(), lowerAnswer.begin(), ::tolower);
            std::string lowerCorrect = correctAnswer;
            std::transform(lowerCorrect.begin(), lowerCorrect.end(), lowerCorrect.begin(), ::tolower);
            
            bool isCorrect = (lowerAnswer == lowerCorrect);
            
            json playerResult;
            playerResult["playerId"] = playerId;
            playerResult["playerName"] = p->name;
            playerResult["answer"] = playerAnswer;
            playerResult["responseTime"] = responseTime / 1000.0; // Convert to seconds
            playerResult["correct"] = isCorrect;
            
            results["results"].push_back(playerResult);
            
            // // Track slowest player (or first incorrect)
            // if (!isCorrect || slowestPlayerId.empty()) {
            //     slowestPlayerId = playerId;
            //     slowestPlayerName = p->name;
            // }

            // Collect incorrect players
            if (!isCorrect) {
                incorrectPlayers.push_back(playerId);
            }
        }
        
        // Check if this is speed_order phase (don't eliminate anyone, just show results)
        std::cout << "🔍 Debug - isSpeedOrderPhase flag: " << (game->isSpeedOrderPhase ? "TRUE" : "FALSE") << std::endl;
        if (game->isSpeedOrderPhase) {
            std::cout << "📋 Speed order question complete, showing results and player order (NO elimination)..." << std::endl;
            
            // Send speed results without elimination
            broadcastToGame(game->pin, results.dump(), nullptr, ctx);
            
            // Mark that we're done with speed order phase
            game->isSpeedOrderPhase = false;
            game->currentQuestion++;  // Move past speed order question
            
            // Clear speed responses for next round
            game->speedResponses.clear();
            return;  // Exit early - no elimination
        }
        
        // Normal speed question - eliminate slowest/incorrect player
        // If no incorrect players, eliminate the slowest
        if (incorrectPlayers.empty()) {
            if (!sortedResponses.empty()) {
                eliminatedId = sortedResponses.back().first;
            }
        }
        // Else eliminate the slowest incorrect player
        else {
            eliminatedId = incorrectPlayers.back();
        }
        // Take eliminated player's name
        if (!eliminatedId.empty()) {
            auto p = findPlayerById(game, eliminatedId);
            if (p) eliminatedName = p->name;
        }
        // Eliminate that player
        if (!eliminatedId.empty()) {
            auto eliminatedPlayer = findPlayerById(game, eliminatedId);
            if (eliminatedPlayer) {
                eliminatedPlayer->isEliminated = true;
                results["eliminated"]["playerId"] = eliminatedId;
                results["eliminated"]["playerName"] = eliminatedName;

                std::cout << "❌ Player eliminated: " << eliminatedName << std::endl;
            }
        }
        
        // Send results with elimination
        broadcastToGame(game->pin, results.dump(), nullptr, ctx);
        
        // Clear speed responses for next round
        game->speedResponses.clear();
    }
}

void handleTiebreakAnswer(struct lws* wsi, const std::string& answer, ServerContext* ctx) {
    auto game = findGameByWsi(wsi, ctx);
    if (!game) return;
    
    auto player = findPlayerByWsi(game, wsi);
    if (!player) return;
    
    // Only tiebreak participants should answer
    auto isTiebreakParticipant = std::find(game->tieBreakerIds.begin(), game->tieBreakerIds.end(), player->id) != game->tieBreakerIds.end();
    if (!isTiebreakParticipant) {
        json response;
        response["type"] = "error";
        response["message"] = "You are not part of the tiebreaker";
        sendToClient(wsi, response.dump());
        return;
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - game->turnStartTime).count();
    
    game->speedResponses[player->id] = {answer, elapsed};
    
    json response;
    response["type"] = "tiebreak_answer_received";
    sendToClient(wsi, response.dump());
    
    std::cout << "📝 Tiebreak answer from " << player->name << ": " << answer << " (" << elapsed << "ms)" << std::endl;
    
    // Check if all tiebreaker participants have answered
    if (game->speedResponses.size() >= game->tieBreakerIds.size()) {
        std::cout << "✅ All tiebreak participants answered, determining winner..." << std::endl;
        
        // Get the correct answer
        std::string correctAnswer = "";
        if (!ctx->speedQuestions.empty()) {
            correctAnswer = ctx->speedQuestions[0].correctAnswer;
        }
        
        // Build results array sorted by response time
        std::vector<std::pair<std::string, std::pair<std::string, long>>> sortedResponses(
            game->speedResponses.begin(), 
            game->speedResponses.end()
        );
        
        std::sort(sortedResponses.begin(), sortedResponses.end(),
            [](const auto& a, const auto& b) {
                return a.second.second < b.second.second; // Sort by response time
            });
        
        json tiebreakResults;
        tiebreakResults["type"] = "tiebreak_results";
        tiebreakResults["results"] = json::array();
        
        // List of incorrect players
        std::vector<std::string> incorrectPlayers;
        std::string eliminatedId;
        std::string eliminatedName;
        
        for (const auto& [playerId, respPair] : sortedResponses) {
            auto p = findPlayerById(game, playerId);
            if (!p) continue;
            
            const std::string& playerAnswer = respPair.first;
            long responseTime = respPair.second;
            
            // Check if answer is correct (case-insensitive)
            std::string lowerAnswer = playerAnswer;
            std::transform(lowerAnswer.begin(), lowerAnswer.end(), lowerAnswer.begin(), ::tolower);
            std::string lowerCorrect = correctAnswer;
            std::transform(lowerCorrect.begin(), lowerCorrect.end(), lowerCorrect.begin(), ::tolower);
            
            bool isCorrect = (lowerAnswer == lowerCorrect);
            
            json playerResult;
            playerResult["playerId"] = playerId;
            playerResult["playerName"] = p->name;
            playerResult["answer"] = playerAnswer;
            playerResult["responseTime"] = responseTime / 1000.0; // Convert to seconds
            playerResult["correct"] = isCorrect;
            
            tiebreakResults["results"].push_back(playerResult);
            
            // Collect incorrect players
            if (!isCorrect) {
                incorrectPlayers.push_back(playerId);
            }
        }
        
        // If no incorrect players, eliminate the slowest
        if (incorrectPlayers.empty()) {
            if (!sortedResponses.empty()) {
                eliminatedId = sortedResponses.back().first;
            }
        }
        // Else eliminate the slowest incorrect player
        else {
            eliminatedId = incorrectPlayers.back();
        }
        
        // Take eliminated player's name
        if (!eliminatedId.empty()) {
            auto p = findPlayerById(game, eliminatedId);
            if (p) eliminatedName = p->name;
        }
        
        // Eliminate that player
        if (!eliminatedId.empty()) {
            auto eliminatedPlayer = findPlayerById(game, eliminatedId);
            if (eliminatedPlayer) {
                eliminatedPlayer->isEliminated = true;
                game->eliminatedPlayers.push_back(eliminatedPlayer);
                game->activePlayers.erase(
                    std::remove(game->activePlayers.begin(), game->activePlayers.end(), eliminatedPlayer),
                    game->activePlayers.end()
                );
                
                tiebreakResults["eliminated"]["playerId"] = eliminatedId;
                tiebreakResults["eliminated"]["playerName"] = eliminatedName;
                
                std::cout << "❌ Tiebreak eliminated: " << eliminatedName << std::endl;
            }
        }
        
        broadcastToGame(game->pin, tiebreakResults.dump(), nullptr, ctx);
        
        // Clear tiebreak state
        game->isTieBreaker = false;
        game->tieBreakerIds.clear();
        game->speedResponses.clear();
        
        // Send round_complete after tiebreaker (same as non-tiebreaker path)
        // json roundEnd;
        // roundEnd["type"] = "round_complete";
        // roundEnd["round"] = game->currentRound;
        // broadcastToGame(game->pin, roundEnd.dump(), nullptr, ctx);
        
        // Don't auto-transition - wait for host to continue
        std::cout << "✅ Tiebreak complete. Waiting for host to continue to Round 2..." << std::endl;
    }
}

void handleContinueFromSpeedOrder(struct lws* wsi, ServerContext* ctx) {
    auto game = findGameByWsi(wsi, ctx);
    if (!game) return;
    
    // Only host can continue
    if (game->hostWsi != wsi) return;
    
    std::cout << "📋 Continuing from speed order to Round 2 question packs..." << std::endl;
    
    // Debug: Show all players and their elimination status
    std::cout << "🔍 Debug - All players in game:" << std::endl;
    for (const auto& p : game->players) {
        std::cout << "  - Player " << p->name << " (ID: " << p->id << ") - Eliminated: " << (p->isEliminated ? "YES" : "NO") << std::endl;
    }
    std::cout << "🔍 Debug - Active players count: " << game->activePlayers.size() << std::endl;
    for (const auto& p : game->activePlayers) {
        std::cout << "  - Active player " << p->name << " (ID: " << p->id << ") - Eliminated: " << (p->isEliminated ? "YES" : "NO") << std::endl;
    }
    
    // First, create player order message based on active players (they should already be in order from speed round)
    json playerOrderMsg;
    playerOrderMsg["type"] = "round2_player_order";
    playerOrderMsg["playerOrder"] = json::array();
    
    // Store player order and initialize turn index
    game->round2PlayerOrder.clear();
    game->round2CurrentTurnIndex = 0;
    game->round2TurnsCompleted = 0; // Initialize turns counter
    game->round2Scores.clear(); // Clear scores
    
    int position = 1;
    for (const auto& player : game->activePlayers) {
        if (!player->isEliminated) {
            json playerInfo;
            playerInfo["position"] = position;
            playerInfo["playerId"] = player->id;
            playerInfo["playerName"] = player->name;
            playerOrderMsg["playerOrder"].push_back(playerInfo);
            game->round2PlayerOrder.push_back(player->id);
            position++;
        }
    }
    
    std::cout << "📋 Player order established with " << game->round2PlayerOrder.size() << " players" << std::endl;
    for (size_t i = 0; i < game->round2PlayerOrder.size(); i++) {
        std::cout << "  Position " << (i+1) << ": Player ID " << game->round2PlayerOrder[i] << std::endl;
    }
    
    broadcastToGame(game->pin, playerOrderMsg.dump(), nullptr, ctx);
    
    // Now send question packs
    json packsMsg;
    packsMsg["type"] = "round2_packs_available";
    packsMsg["packs"] = json::array();
    
    for (const auto& pack : ctx->questionPacks) {
        json packInfo;
        packInfo["id"] = pack.id;
        packInfo["title"] = pack.title;
        packInfo["description"] = pack.description;
        packInfo["questionCount"] = pack.questions.size();
        packsMsg["packs"].push_back(packInfo);
    }
    
    packsMsg["currentTurnIndex"] = game->round2CurrentTurnIndex;
    
    broadcastToGame(game->pin, packsMsg.dump(), nullptr, ctx);
}

void handleContinueToRound2(struct lws* wsi, ServerContext* ctx) {
    auto game = findGameByWsi(wsi, ctx);
    if (!game) return;
    
    // Only host can continue
    if (game->hostWsi != wsi) return;
    
    std::cout << "📋 Host starting Round 2 with speed order question..." << std::endl;
    
    game->currentRound = 2;
    game->currentQuestion = 0;
    game->questionsPerRound = 1; // Only speed order question
    game->isSpeedOrderPhase = true;  // Mark this as speed order phase
    
    // Reset player states for Round 2
    for (auto& p : game->activePlayers) {
        p->hasAnswered = false;
        p->roundScore = 0;
    }
    
    game->answers.clear();
    game->speedResponses.clear();
    game->turnStartTime = std::chrono::steady_clock::now();
    
    // Announce Round 2 start
    json round2Start;
    round2Start["type"] = "round2_start";
    round2Start["message"] = "Round 2: Determine Player Order";
    round2Start["phase"] = "speed_order";
    broadcastToGame(game->pin, round2Start.dump(), nullptr, ctx);
    
    // Send speed order question only to active players
    if (!ctx->speedQuestions.empty()) {
        const auto& sq = ctx->speedQuestions[0];
        json speedQ;
        speedQ["type"] = "speed_question";
        speedQ["question"]["id"] = sq.id;
        speedQ["question"]["text"] = sq.text;
        speedQ["phase"] = "speed_order";
        broadcastToActivePlayers(game->pin, speedQ.dump(), nullptr, ctx);
    }
}

void handleDisconnection(struct lws* wsi, ServerContext* ctx) {
    auto game = findGameByWsi(wsi, ctx);
    if (!game) return;
    
    auto player = findPlayerByWsi(game, wsi);
    if (player) {
        player->connected = false;
        player->wsi = nullptr;
        
        json disconnect;
        disconnect["type"] = "player_disconnected";
        disconnect["playerId"] = player->id;
        disconnect["playerName"] = player->name;
        
        broadcastToGame(game->pin, disconnect.dump(), wsi, ctx);
    }
    
    ctx->wsToGamePin.erase(wsi);
    ctx->wsToPlayerId.erase(wsi);
}

// Placeholder implementations for remaining handlers
void handleQuestionPackSelection(struct lws* wsi, const std::string& packId, ServerContext* ctx) {
    auto game = findGameByWsi(wsi, ctx);
    if (!game) return;
    
    auto player = findPlayerByWsi(game, wsi);
    if (!player) return;
    
    // Check if this pack was already selected
    if (std::find(game->selectedPacks.begin(), game->selectedPacks.end(), packId) != game->selectedPacks.end()) {
        json error;
        error["type"] = "error";
        error["message"] = "This pack has already been selected";
        sendToClient(wsi, error.dump());
        return;
    }
    
    // Find the pack
    auto packIt = std::find_if(ctx->questionPacks.begin(), ctx->questionPacks.end(),
        [&packId](const QuestionPack& p) { return p.id == packId; });
    
    if (packIt == ctx->questionPacks.end()) {
        json error;
        error["type"] = "error";
        error["message"] = "Pack not found";
        sendToClient(wsi, error.dump());
        return;
    }
    
    // Mark pack as selected
    game->selectedPacks.push_back(packId);
    game->currentPack = std::make_shared<QuestionPack>(*packIt);
    game->currentPackQuestionIndex = 0;
    game->currentPackScore = 0;
    game->currentPackPlayerId = player->id;
    game->turnStartTime = std::chrono::steady_clock::now();
    
    std::cout << "📦 Player " << player->name << " selected pack: " << packIt->title << std::endl;
    
    // Send pack selected message to all players
    json packSelectedMsg;
    packSelectedMsg["type"] = "pack_selected";
    packSelectedMsg["packId"] = packId;
    packSelectedMsg["packTitle"] = packIt->title;
    packSelectedMsg["playerName"] = player->name;
    packSelectedMsg["playerId"] = player->id;
    broadcastToGame(game->pin, packSelectedMsg.dump(), nullptr, ctx);
    
    // Send all pack questions to the game immediately
    json questionsMsg;
    questionsMsg["type"] = "pack_questions";
    questionsMsg["packTitle"] = packIt->title;
    questionsMsg["questions"] = json::array();
    questionsMsg["timeLimit"] = 45; // 45 seconds total
    questionsMsg["currentPlayer"] = player->name;
    
    // Read the pack data from JSON to get answers
    std::ifstream packFile("questions/round2-question-packs.json");
    if (packFile.is_open()) {
        json packsJson;
        packFile >> packsJson;
        
        // Find the matching pack in JSON
        for (const auto& jsonPack : packsJson) {
            if (jsonPack["id"] == packId) {
                // Use questions from JSON which have the answer field
                for (const auto& q : jsonPack["questions"]) {
                    json questionData;
                    questionData["id"] = q["id"];
                    questionData["text"] = q["text"];
                    questionData["answer"] = q["answer"];
                    questionsMsg["questions"].push_back(questionData);
                }
                break;
            }
        }
    }
    
    broadcastToGame(game->pin, questionsMsg.dump(), nullptr, ctx);
}

void handleStartPackQuestions(struct lws* wsi, ServerContext* ctx) {
    auto game = findGameByWsi(wsi, ctx);
    if (!game) return;
    
    // Only host can start pack questions
    if (game->hostWsi != wsi) return;
    
    if (!game->currentPack) {
        json error;
        error["type"] = "error";
        error["message"] = "No pack selected";
        sendToClient(wsi, error.dump());
        return;
    }
    
    std::cout << "▶️ Host starting questions for pack: " << game->currentPack->title << std::endl;
    
    // Reset timer
    game->turnStartTime = std::chrono::steady_clock::now();
    
    // Send all pack questions to the game
    json questionsMsg;
    questionsMsg["type"] = "pack_questions";
    questionsMsg["packTitle"] = game->currentPack->title;
    questionsMsg["questions"] = json::array();
    questionsMsg["timeLimit"] = 45; // 45 seconds total
    
    // Read the pack data from JSON to get answers
    std::ifstream packFile("questions/round2-question-packs.json");
    if (packFile.is_open()) {
        json packsJson;
        packFile >> packsJson;
        
        // Find the matching pack in JSON
        for (const auto& jsonPack : packsJson) {
            if (jsonPack["id"] == game->currentPack->id) {
                // Use questions from JSON which have the answer field
                for (const auto& q : jsonPack["questions"]) {
                    json questionData;
                    questionData["id"] = q["id"];
                    questionData["text"] = q["text"];
                    questionData["answer"] = q["answer"];
                    questionsMsg["questions"].push_back(questionData);
                }
                break;
            }
        }
    }
    
    broadcastToGame(game->pin, questionsMsg.dump(), nullptr, ctx);
}

void handlePackAnswerVerified(struct lws* wsi, bool isCorrect, int questionIndex, ServerContext* ctx) {
    auto game = findGameByWsi(wsi, ctx);
    if (!game) return;
    
    // Only host can verify answers
    if (game->hostWsi != wsi) return;
    
    if (!game->currentPack) return;
    
    std::cout << "✅ Host verified answer: " << (isCorrect ? "Correct" : "Incorrect") << std::endl;
    
    // Update score
    if (isCorrect) {
        game->currentPackScore++;
    }
    
    // Broadcast verification to all players
    json verifyMsg;
    verifyMsg["type"] = "pack_answer_verified";
    verifyMsg["isCorrect"] = isCorrect;
    verifyMsg["questionIndex"] = questionIndex;
    verifyMsg["currentScore"] = game->currentPackScore;
    broadcastToGame(game->pin, verifyMsg.dump(), nullptr, ctx);
    
    // Check if all questions answered
    if (questionIndex + 1 >= static_cast<int>(game->currentPack->questions.size())) {
        std::cout << "🎯 Pack complete! Final score: " << game->currentPackScore << "/" << game->currentPack->questions.size() << std::endl;
        
        // Find current player and add score to their Round 2 total
        auto player = findPlayerById(game, game->currentPackPlayerId);
        std::string playerName = player ? player->name : "Player";
        
        // Add to player's Round 2 score
        if (game->round2Scores.find(game->currentPackPlayerId) == game->round2Scores.end()) {
            game->round2Scores[game->currentPackPlayerId] = 0;
        }
        game->round2Scores[game->currentPackPlayerId] += game->currentPackScore;
        
        std::cout << "📊 " << playerName << "'s Round 2 total: " << game->round2Scores[game->currentPackPlayerId] << std::endl;
        
        // Broadcast pack completion
        json completeMsg;
        completeMsg["type"] = "pack_complete";
        completeMsg["playerName"] = playerName;
        completeMsg["score"] = game->currentPackScore;
        completeMsg["totalQuestions"] = game->currentPack->questions.size();
        broadcastToGame(game->pin, completeMsg.dump(), nullptr, ctx);
        
        // Reset pack state
        game->currentPack = nullptr;
        game->currentPackScore = 0;
        game->currentPackPlayerId = "";
    }
}

void handleEndTurn(struct lws* wsi, ServerContext* ctx) {
    std::cout << "🔍 handleEndTurn called" << std::endl;
    
    auto game = findGameByWsi(wsi, ctx);
    if (!game) {
        std::cout << "❌ Game not found in handleEndTurn" << std::endl;
        return;
    }
    
    // Only host can end turn
    if (game->hostWsi != wsi) {
        std::cout << "❌ Only host can end turn" << std::endl;
        return;
    }
    
    std::cout << "⏹️ Host ended current player's turn" << std::endl;
    
    // Increment turns completed
    game->round2TurnsCompleted++;
    
    // Check if 2 full cycles are complete (each player gets 2 turns)
    int totalTurnsNeeded = game->round2PlayerOrder.size() * 2;
    
    std::cout << "📊 Turns completed: " << game->round2TurnsCompleted << "/" << totalTurnsNeeded << std::endl;
    
    if (game->round2TurnsCompleted >= totalTurnsNeeded) {
        std::cout << "🏁 Round 2 complete! Calculating winners..." << std::endl;
        
        // Find highest score
        int highestScore = 0;
        for (const auto& entry : game->round2Scores) {
            if (entry.second > highestScore) {
                highestScore = entry.second;
            }
        }
        
        // Find all players with highest score
        std::vector<std::string> winners;
        for (const auto& entry : game->round2Scores) {
            if (entry.second == highestScore) {
                auto player = findPlayerById(game, entry.first);
                if (player) {
                    winners.push_back(player->name);
                }
            }
        }
        
        // Broadcast game over with winners
        json gameOverMsg;
        gameOverMsg["type"] = "game_over";
        gameOverMsg["winners"] = json::array();
        for (const auto& winner : winners) {
            gameOverMsg["winners"].push_back(winner);
        }
        gameOverMsg["finalScores"] = json::array();
        for (const auto& entry : game->round2Scores) {
            auto player = findPlayerById(game, entry.first);
            if (player) {
                json scoreInfo;
                scoreInfo["playerName"] = player->name;
                scoreInfo["score"] = entry.second;
                gameOverMsg["finalScores"].push_back(scoreInfo);
            }
        }
        
        std::cout << "🏆 Winners: ";
        for (size_t i = 0; i < winners.size(); i++) {
            std::cout << winners[i];
            if (i < winners.size() - 1) std::cout << ", ";
        }
        std::cout << " with score: " << highestScore << std::endl;
        
        broadcastToGame(game->pin, gameOverMsg.dump(), nullptr, ctx);
        return;
    }
    
    // Don't send turn_ended message - just move to next player
    // The round2_packs_available message will update the UI
    
    // Get remaining active players
    std::vector<std::shared_ptr<Player>> activePlayers;
    for (const auto& p : game->activePlayers) {
        if (!p->isEliminated) {
            activePlayers.push_back(p);
        }
    }
    
    // Check if all packs are selected or all players have had turns
    if (game->selectedPacks.size() >= ctx->questionPacks.size()) {
        // Round 2 complete
        json round2CompleteMsg;
        round2CompleteMsg["type"] = "round2_complete";
        round2CompleteMsg["message"] = "Round 2 Complete!";
        broadcastToGame(game->pin, round2CompleteMsg.dump(), nullptr, ctx);
        return;
    }
    
    // Send updated packs with next player's turn
    if (!game->round2PlayerOrder.empty()) {
        std::cout << "📍 Current turn index before increment: " << game->round2CurrentTurnIndex << std::endl;
        game->round2CurrentTurnIndex = (game->round2CurrentTurnIndex + 1) % game->round2PlayerOrder.size();
        std::cout << "📍 Moving to turn index: " << game->round2CurrentTurnIndex << " (Player ID: " << game->round2PlayerOrder[game->round2CurrentTurnIndex] << ")" << std::endl;
    }
    
    json packsMsg;
    packsMsg["type"] = "round2_packs_available";
    packsMsg["packs"] = json::array();
    
    for (const auto& pack : ctx->questionPacks) {
        json packInfo;
        packInfo["id"] = pack.id;
        packInfo["title"] = pack.title;
        packInfo["description"] = pack.description;
        packInfo["questionCount"] = pack.questions.size();
        
        // Mark if already selected
        bool isSelected = std::find(game->selectedPacks.begin(), game->selectedPacks.end(), pack.id) != game->selectedPacks.end();
        if (isSelected) {
            packInfo["selected"] = true;
        }
        
        packsMsg["packs"].push_back(packInfo);
    }
    
    packsMsg["currentTurnIndex"] = game->round2CurrentTurnIndex;
    
    broadcastToGame(game->pin, packsMsg.dump(), nullptr, ctx);
}

void handleHostDecision(struct lws* wsi, bool givePoints, ServerContext* ctx) {
    // TODO: Implement host decision logic
}

void handleEndGame(struct lws* wsi, ServerContext* ctx) {
    auto game = findGameByWsi(wsi, ctx);
    if (!game) return;
    
    if (game->hostWsi != wsi) return;
    
    json endMsg;
    endMsg["type"] = "game_ended";
    broadcastToGame(game->pin, endMsg.dump(), nullptr, ctx);
    
    ctx->games.erase(game->pin);
}
