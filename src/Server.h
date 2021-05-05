//
// Created by Maria.Filipanova on 5/5/21.
//

#ifndef MY_SCREEN_CLIENT_H
#define MY_SCREEN_MESSENGER_H

#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>
#include <sys/fcntl.h>
#include "../common/Common.h"
#include "ServerLogic.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>

//pass data to server from it's children
#define INTERNAL_STDIN 0x10
#define INTERNAL_STDOUT 0x11
#define INTERNAL_STDERR 0x12
#define INTERNAL_QUEUE_NAME "/internalKek"

class Server {
public:
    Server();

    void greetClient(std::string sessionId);

    void sendSTDOUT(const std::string &msg);

    void sendSTDERR(const std::string &msg);

    void listSessions(const std::vector<std::string> &ids);

    void acceptMessages();

    ~Server();

private:
    mqd_t mq;
    ServerLogic logic;
    //this one is for receiving data from children
    mqd_t internal_mq;
};


#endif //MY_SCREEN_CLIENT_H
