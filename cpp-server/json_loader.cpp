#include "game_server.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "json.hpp"  // We'll use nlohmann/json for JSON parsing

using json = nlohmann::json;

bool loadQuestionsFromJSON(const std::string& filename, std::vector<Question>& questions) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << " Could not open " << filename << std::endl;
            return false;
        }
        
        json j;
        file >> j;
        
        for (const auto& item : j) {
            Question q;
            q.id = item["id"];
            q.text = item["text"];
            q.options = item["options"].get<std::vector<std::string>>();
            q.correctAnswer = item["correctAnswer"];
            q.timeLimit = item["timeLimit"];
            questions.push_back(q);
        }
        
        std::cout << " Loaded " << questions.size() << " questions from " << filename << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << " Error loading questions from " << filename << ": " << e.what() << std::endl;
        return false;
    }
}

bool loadSpeedQuestionsFromJSON(const std::string& filename, std::vector<SpeedQuestion>& questions) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << " Could not open " << filename << std::endl;
            return false;
        }
        
        json j;
        file >> j;
        
        for (const auto& item : j) {
            SpeedQuestion q;
            q.id = item["id"];
            q.text = "Type the answer: " + item["question"].get<std::string>();
            q.correctAnswer = item["correctAnswer"];
            questions.push_back(q);
        }
        
        std::cout << " Loaded " << questions.size() << " speed questions from " << filename << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << " Error loading speed questions from " << filename << ": " << e.what() << std::endl;
        return false;
    }
}

bool loadQuestionPacksFromJSON(const std::string& filename, std::vector<QuestionPack>& packs) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << " Could not open " << filename << std::endl;
            return false;
        }
        
        json j;
        file >> j;
        
        for (const auto& pack : j) {
            QuestionPack qp;
            qp.id = pack["id"];
            qp.title = pack["title"];
            qp.description = pack["description"];
            
            for (const auto& q : pack["questions"]) {
                Question question;
                question.id = 0; // Pack questions don't use numeric IDs
                question.text = q["text"];
                question.correctAnswer = -1; // Text answer, not multiple choice
                question.timeLimit = 45; // Default time limit
                qp.questions.push_back(question);
            }
            
            packs.push_back(qp);
        }
        
        std::cout << " Loaded " << packs.size() << " question packs from " << filename << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << " Error loading question packs from " << filename << ": " << e.what() << std::endl;
        return false;
    }
}

// Create default questions if files don't exist
void createDefaultQuestions(std::vector<Question>& questions) {
    questions = {
        {1, "What is the capital of France?", {"London", "Berlin", "Paris", "Madrid"}, 2, 15},
        {2, "Which planet is known as the Red Planet?", {"Venus", "Mars", "Jupiter", "Saturn"}, 1, 15},
        {3, "What is 2 + 2?", {"3", "4", "5", "6"}, 1, 10},
        {4, "Who painted the Mona Lisa?", {"Van Gogh", "Da Vinci", "Picasso", "Monet"}, 1, 15}
    };
}

void createDefaultSpeedQuestions(std::vector<SpeedQuestion>& questions) {
    questions = {
        {"speed1", "Type the number: What is 7  8?", "56"},
        {"speed2", "Type the city: Capital of Japan?", "tokyo"},
        {"speed3", "Type the number: What is 10 + 15?", "25"}
    };
}

void createDefaultQuestionPacks(std::vector<QuestionPack>& packs) {
    QuestionPack pack1;
    pack1.id = "pack1";
    pack1.title = " Geography Masters";
    pack1.description = "World capitals, countries, and landmarks";
    pack1.questions = {
        {0, "What is the capital of Australia?", {}, -1, 45},
        {0, "Which country has the most islands?", {}, -1, 45}
    };
    
    packs.push_back(pack1);
}

