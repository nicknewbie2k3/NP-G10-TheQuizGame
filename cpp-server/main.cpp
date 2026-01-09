#include <libwebsockets.h>
#include <string>
#include <iostream>
#include <signal.h>
#include "game_server.h"
#include "json.hpp"

using json = nlohmann::json;

static int interrupted = 0;
static ServerContext* globalContext = nullptr;

// Signal handler for clean shutdown
void sigint_handler(int sig) {
    interrupted = 1;
}

// WebSocket protocol callback
static int callback_game_protocol(struct lws *wsi, enum lws_callback_reasons reason,
                                   void *user, void *in, size_t len) {
    
    ServerContext* ctx = (ServerContext*)lws_context_user(lws_get_context(wsi));
    
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            std::cout << " New WebSocket connection established" << std::endl;
            break;
            
        case LWS_CALLBACK_RECEIVE: {
            try {
                std::string message((char*)in, len);
                json msg = json::parse(message);
                
                std::string type = msg["type"];
                std::cout << " Received message: " << type << std::endl;
                
                if (type == "create_game") {
                    handleCreateGame(wsi, ctx);
                }
                else if (type == "join_game") {
                    handleJoinGame(wsi, msg["gamePin"], msg["playerName"], ctx);
                }
                else if (type == "start_game") {
                    handleStartGame(wsi, ctx);
                }
                else if (type == "submit_answer") {
                    handleSubmitAnswer(wsi, msg["questionId"], msg["answer"], ctx);
                }
                else if (type == "submit_speed_answer") {
                    handleSpeedAnswer(wsi, msg["questionId"], msg["answer"], ctx);
                }
                else if (type == "submit_tiebreak_answer") {
                    handleTiebreakAnswer(wsi, msg["answer"], ctx);
                }
                else if (type == "continue_to_round2") {
                    handleContinueToRound2(wsi, ctx);
                }
                else if (type == "continue_from_speed_order") {
                    handleContinueFromSpeedOrder(wsi, ctx);
                }
                else if (type == "select_question_pack") {
                    handleQuestionPackSelection(wsi, msg["packId"], ctx);
                }
                else if (type == "start_pack_questions") {
                    handleStartPackQuestions(wsi, ctx);
                }
                else if (type == "submit_pack_answer") {
                    handleSubmitPackAnswer(wsi, msg["answer"], msg["questionIndex"], ctx);
                }
                else if (type == "pack_answer_verified") {
                    handlePackAnswerVerified(wsi, msg["isCorrect"], msg["questionIndex"], ctx);
                }
                else if (type == "end_pack_early") {
                    handleEndPackEarly(wsi, ctx);
                }
                else if (type == "end_turn") {
                    handleEndTurn(wsi, ctx);
                }
                else if (type == "leave_game") {
                    handleLeaveGame(wsi, ctx);
                }
                else if (type == "host_decision") {
                    handleHostDecision(wsi, msg["givePoints"], ctx);
                }
                else if (type == "next_question") {
                    handleNextQuestion(wsi, ctx);
                }
                else if (type == "next_round") {
                    handleNextRound(wsi, ctx);
                }
                else if (type == "end_game") {
                    handleEndGame(wsi, ctx);
                }
                else {
                    std::cout << " Unknown message type: " << type << std::endl;
                }
                
            } catch (const std::exception& e) {
                std::cerr << " Error parsing message: " << e.what() << std::endl;
            }
            break;
        }
            
        case LWS_CALLBACK_CLOSED:
            std::cout << " WebSocket connection closed" << std::endl;
            handleDisconnection(wsi, ctx);
            break;
            
        default:
            break;
    }
    
    return 0;
}

// Protocol definition
static struct lws_protocols protocols[] = {
    {
        "game-protocol",           // Protocol name
        callback_game_protocol,    // Callback function
        0,                        // Per-session data size
        1024,                     // RX buffer size
    },
    { NULL, NULL, 0, 0 } // Terminator
};

int main(int argc, char **argv) {
    struct lws_context_creation_info info;
    struct lws_context *context;
    const char *interface = NULL;
    int port = 8080;
    int logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;
    
    // Set up signal handler
    signal(SIGINT, sigint_handler);
    
    // Initialize server context
    ServerContext serverCtx;
    globalContext = &serverCtx;
    
    // Load questions from JSON files
    std::cout << " Loading questions..." << std::endl;
    
    if (!loadQuestionsFromJSON("questions/round1-questions.json", serverCtx.mockQuestions)) {
        std::cout << " Using default Round 1 questions" << std::endl;
        createDefaultQuestions(serverCtx.mockQuestions);
    }
    
    if (!loadSpeedQuestionsFromJSON("questions/speed-questions.json", serverCtx.speedQuestions)) {
        std::cout << " Using default speed questions" << std::endl;
        createDefaultSpeedQuestions(serverCtx.speedQuestions);
    }
    
    if (!loadQuestionPacksFromJSON("questions/round2-question-packs.json", serverCtx.questionPacks)) {
        std::cout << " Using default question packs" << std::endl;
        createDefaultQuestionPacks(serverCtx.questionPacks);
    }
    
    // Set log level
    lws_set_log_level(logs, NULL);
    
    // Initialize context creation info
    memset(&info, 0, sizeof(info));
    info.port = port;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    info.user = &serverCtx;
    
    // Create libwebsocket context
    context = lws_create_context(&info);
    if (!context) {
        std::cerr << " Failed to create libwebsocket context" << std::endl;
        return 1;
    }
    
    std::cout << " WebSocket Game Server started on port " << port << std::endl;
    std::cout << " Players can connect to ws://localhost:" << port << std::endl;
    std::cout << "Press Ctrl+C to stop the server" << std::endl;
    
    // Main event loop
    int n = 0;
    while (n >= 0 && !interrupted) {
        n = lws_service(context, 50); // 50ms timeout
    }
    
    // Cleanup
    lws_context_destroy(context);
    
    std::cout << "\n Server stopped" << std::endl;
    
    return 0;
}

