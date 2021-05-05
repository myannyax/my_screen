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
#include <sys/fcntl.h>
#include "../../common/Common.h"
#include "ClientLogic.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>


class Client {
public:
    Client();

    void newSession(std::string sessionId);

    void attach(std::string sessionId);

    void sendSTDIN(const std::string &msg);

    void detach();

    void kill(std::string sessionId);

    void list();

    void acceptMessage();

    ~Client();

private:
    mqd_t mq;
    ClientLogic logic;
};


#endif //MY_SCREEN_CLIENT_H
