//
// Created by Maria.Filipanova on 5/5/21.
//

#ifndef MY_SCREEN_SERVERLOGIC_H
#define MY_SCREEN_SERVERLOGIC_H

#include <string>
#include <vector>
#include <iostream>

struct BuffEntry {
    bool isErr;
    std::string data;
};


class ServerLogic {
public:
    //todo gen id if empty
    std::string createNewSession(const std::string& sessionId);

    void attachClientToSession(const std::string& sessionId);

    std::vector<BuffEntry> getSessionBuffer(const std::string& sessionId);

    void detachClient();

    void killSession(const std::string& sessionId);

    std::vector<std::string> getSessionIds();

    void receiveSTDIN(const std::string& msg);

private:

};


#endif //MY_SCREEN_SERVERLOGIC_H
