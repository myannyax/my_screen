#ifndef MY_SCREEN_SERVER_H
#define MY_SCREEN_SERVER_H

#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <unordered_map>

#include "Common.h"
#include "ServerLogic.h"

class Server {
public:
    Server();

    void createNewSession(const std::string& id,
                          const std::string& outputQueueName);

    void attachClientToSession(const std::string& sessionId,
                               const std::string& outputQueueName);

    static void listSessions(const std::vector<std::string> &ids, const std::string& outputQueueName);

    void killSession(const std::string& sessionId, const std::string& outputQueueName);

    void endSession(const std::string& sessionId);

    void acceptMessages();

    ~Server();

private:
    mqd_t inputQueue;
    std::unordered_map<std::string, mqd_t> sessionQueues;
    ServerLogic logic;
};

#endif //MY_SCREEN_SERVER_H
