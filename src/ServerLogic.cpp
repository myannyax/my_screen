//
// Created by Maria.Filipanova on 5/5/21.
//

#include "ServerLogic.h"

std::string ServerLogic::createNewSession(const std::string& sessionId) {
    //TODO
    std::cout << "new session created\n";
    return "kek";
}

void ServerLogic::attachClientToSession(const std::string& sessionId) {
    //TODO
    std::cout << "client attached\n";
}

std::vector<BuffEntry> ServerLogic::getSessionBuffer(const std::string& sessionId) {
    //TODO
    std::vector<BuffEntry> res;
    BuffEntry kek1;
    BuffEntry kek2;
    kek1.isErr = true;
    kek1.data = "haha kekkek";
    kek1.isErr = false;
    kek1.data = "haha kekkek\nhaha kekkek\nhaha kekkek";
    res.emplace_back(kek1);
    res.emplace_back(kek2);
    return std::vector<BuffEntry>();
}

void ServerLogic::detachClient() {
    //TODO
    std::cout << "client detached\n";
}

void ServerLogic::killSession(const std::string& sessionId) {
    //TODO
}

std::vector<std::string> ServerLogic::getSessionIds() {
    //TODO
    return std::vector<std::string>();
}

void ServerLogic::receiveSTDIN(const std::string& msg) {
    //TODO
    std::cout << msg << std::endl;
}
