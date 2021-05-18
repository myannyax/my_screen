//
// Created by Maria.Filipanova on 5/5/21.
//

#ifndef MY_SCREEN_CLIENT_H
#define MY_SCREEN_CLIENT_H

#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>
#include <string>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <thread>

#include "Common.h"

class Client {
private:
    std::string sessionId;

    std::string outputQueueName;

    mqd_t serverQueue;
    mqd_t inputQueue = 0;
    mqd_t outputQueue;

    pid_t inputPid = 0;

public:
    Client();

    ~Client();

    std::string newSession(const std::string& id = "");

    void attach(const std::string& newSessionId);

    void sendInput(const std::string &msg) const;

    void detach() const;

    void kill(const std::string &id);

    void list();

    void shutdown();

    void acceptMessages() const;

    void handleInput() const;

    void start();
};


#endif //MY_SCREEN_CLIENT_H
