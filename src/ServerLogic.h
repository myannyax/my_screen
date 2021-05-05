//
// Created by Maria.Filipanova on 5/5/21.
//

#ifndef MY_SCREEN_SERVERLOGIC_H
#define MY_SCREEN_SERVERLOGIC_H

#include <string>
#include <vector>

struct BuffEntry {
    bool isErr;
    std::string data;
};


class ServerLogic {
public:
    //todo gen id if empty
    std::string createNewSession(std::string sessionId);
    void attachClientToSession(std::string sessionId);
    std::vector<BuffEntry> getSessionBuffer(std::string sessionId);
    void detachClient();
    void killSession(std::string sessionId);
    std::vector<std::string> getSessionIds();
    void receiveSTDIN(std::string msg);

private:

};


#endif //MY_SCREEN_SERVERLOGIC_H
