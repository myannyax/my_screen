//
// Created by Maria.Filipanova on 5/5/21.
//

#ifndef MY_SCREEN_SERVERLOGIC_H
#define MY_SCREEN_SERVERLOGIC_H

#include <string>
#include <vector>
#include <iostream>
#include <unordered_set>
#include <unordered_map>

#include "Common.h"

class ServerLogic {
private:
    class IdContainer {
    private:
        std::unordered_set<std::string> ids;
        std::string prefix;
        int counter = 0;

    public:
        explicit IdContainer(std::string prefix);
        bool insert(const std::string& id);
        bool remove(const std::string& id);
        std::string generateId();

        const std::unordered_set<std::string>& get_ids();
    };

public:
    std::string createNewSession(std::string sessionId = "");

    void killSession(const std::string& sessionId);

    std::vector<std::string> getSessionIds();

private:
    IdContainer sessionIds{"session_"};
    std::unordered_map<std::string, mqd_t> sessionQueues;
};


#endif //MY_SCREEN_SERVERLOGIC_H
