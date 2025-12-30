#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <libwebsockets.h>

// Forward declarations
struct Player;
struct Game;
struct Question;

// Question structure
struct Question {
    int id;
    std::string text;
    std::vector<std::string> options;
    int correctAnswer;
    int timeLimit;
};

// Speed Question structure
struct SpeedQuestion {
    std::string id;
    std::string text;
    std::string correctAnswer;
};

// Question Pack structure
struct QuestionPack {
    std::string id;
    std::string title;
    std::string description;
    std::vector<Question> questions;
};

// Player structure
struct Player {
    std::string id;
    std::string name;
    struct lws* wsi;
    bool connected;
    bool hasAnswered;
    bool isEliminated;
    int score;
    int roundScore;
    std::chrono::steady_clock::time_point answerTime;
};

// Game structure
struct Game {
    std::string pin;
    struct lws* hostWsi;
    std::vector<std::shared_ptr<Player>> players;
    std::vector<std::shared_ptr<Player>> activePlayers;
    std::vector<std::shared_ptr<Player>> eliminatedPlayers;
    
    // Game state
    std::string gameState; // "lobby", "round1", "round2", "finished"
    int currentRound;
    int currentQuestion;
    int questionsPerRound;
    
    // Round 1
    std::vector<Question> questions;
    std::map<std::string, int> scores;
    std::map<std::string, int> roundScores;
    std::map<std::string, int> answers;
    
    // Round 2 - Turn-based system
    std::vector<std::string> turnOrder;
    int currentPlayerTurn;
    bool isSpeedPhase;
    std::map<std::string, std::pair<std::string, long>> speedResponses;
    std::vector<QuestionPack> questionPacks;
    std::shared_ptr<QuestionPack> currentPack;
    int currentPackQuestionIndex;
    bool waitingForHost;
    std::map<std::string, std::vector<int>> turnScores;
    int currentTurnNumber;
    std::vector<std::string> selectedPacks;
    
    // Timing
    std::chrono::steady_clock::time_point turnStartTime;
    int turnTimeRemaining;
};

// Server context
struct ServerContext {
    std::map<std::string, std::shared_ptr<Game>> games;
    std::map<struct lws*, std::string> wsToPlayerId;
    std::map<struct lws*, std::string> wsToGamePin;
    
    // Questions loaded from JSON
    std::vector<Question> mockQuestions;
    std::vector<SpeedQuestion> speedQuestions;
    std::vector<QuestionPack> questionPacks;
};

// Function declarations
std::string generateGamePin();
void broadcastToGame(const std::string& gamePin, const std::string& message, struct lws* excludeWsi, ServerContext* ctx);
void sendToClient(struct lws* wsi, const std::string& message);
std::shared_ptr<Player> findPlayerByWsi(std::shared_ptr<Game> game, struct lws* wsi);
std::shared_ptr<Game> findGameByWsi(struct lws* wsi, ServerContext* ctx);

// Message handlers
void handleCreateGame(struct lws* wsi, ServerContext* ctx);
void handleJoinGame(struct lws* wsi, const std::string& gamePin, const std::string& playerName, ServerContext* ctx);
void handleStartGame(struct lws* wsi, ServerContext* ctx);
void handleSubmitAnswer(struct lws* wsi, int questionId, int answer, ServerContext* ctx);
void handleSpeedAnswer(struct lws* wsi, const std::string& questionId, const std::string& answer, ServerContext* ctx);
void handleQuestionPackSelection(struct lws* wsi, const std::string& packId, ServerContext* ctx);
void handleHostDecision(struct lws* wsi, bool givePoints, ServerContext* ctx);
void handleNextQuestion(struct lws* wsi, ServerContext* ctx);
void handleNextRound(struct lws* wsi, ServerContext* ctx);
void handleEndGame(struct lws* wsi, ServerContext* ctx);
void handleDisconnection(struct lws* wsi, ServerContext* ctx);

// Question loading
bool loadQuestionsFromJSON(const std::string& filename, std::vector<Question>& questions);
bool loadSpeedQuestionsFromJSON(const std::string& filename, std::vector<SpeedQuestion>& questions);
bool loadQuestionPacksFromJSON(const std::string& filename, std::vector<QuestionPack>& packs);

// Default question creators
void createDefaultQuestions(std::vector<Question>& questions);
void createDefaultSpeedQuestions(std::vector<SpeedQuestion>& questions);
void createDefaultQuestionPacks(std::vector<QuestionPack>& packs);

// JSON utilities
std::string createJSONMessage(const std::string& type, const std::string& data);

#endif // GAME_SERVER_H
