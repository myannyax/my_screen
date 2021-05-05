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

//pass data to server from it's children
#define INTERNAL_STDIN 0x03
#define INTERNAL_STDOUT 0x04

class Server {
public:
    Server() {
        mq = mq_open(QUEUE_NAME, O_WRONLY);
        internal_mq = mq_open(INTERNAL_QUEUE_NAME.c_str(), O_WRONLY);
        CHECK((mqd_t)-1 != mq);
        CHECK((mqd_t)-1 != internal_mq);
    }
    void greetClient(unsigned int sessionId);

    void sendSTDOUT(const std::string &msg);

    void sendSTDERR(const std::string &msg);

    void listSessions(const std::vector<unsigned int>& ids);

    void acceptMessage();

    ~Server() {
        CHECK((mqd_t)-1 != mq_close(mq));
    }


private:
    mqd_t mq;
    ServerLogic logic;
    //this one is for receiving data from children
    std::string INTERNAL_QUEUE_NAME = "internal_kek";
    mqd_t internal_mq;
};


#endif //MY_SCREEN_CLIENT_H
