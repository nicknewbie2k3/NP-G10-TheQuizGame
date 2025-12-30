#include "game_server.h"
#include <random>
#include <algorithm>
#include <sstream>
#include <ctime>
#include <iostream>
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

std::shared_ptr<Player> findPlayerByWsi(std::shared_ptr<Game> game, struct lws* wsi) {
    for (const auto& player : game->players) {
        if (player->wsi == wsi) {
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
            
            for (const auto& p : game->activePlayers) {
                if (!p->isEliminated && p->roundScore < lowestScore) {
                    lowestScore = p->roundScore;
                    lowestPlayer = p;
                }
            }
            
            if (lowestPlayer) {
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
        // Start Round 2 - Speed Question Phase
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
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - game->turnStartTime).count();
    
    game->speedResponses[player->id] = {answer, elapsed};
    
    json response;
    response["type"] = "speed_answer_received";
    sendToClient(wsi, response.dump());
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
    // TODO: Implement pack selection logic
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
