//
// Created by Maria.Filipanova on 5/5/21.
//

#ifndef MY_SCREEN_COMMON_H
#define MY_SCREEN_COMMON_H

#include <string>
#include <mqueue.h>
#include <cstring>

#define NEW_SESSION_CODE 0x00
#define ATTACH_CODE 0x01
#define SERVER_HELLO_CODE 0x02
#define STDIN_CODE 0x03
#define STDOUT_CODE 0x04
#define DETACH_CODE 0x06
#define KILL_CODE 0x07
#define LIST_CODE 0x08
#define SESSIONS_CODE 0x09
#define DETACHED_CODE 0x0A
#define TERMINATED_CODE 0x0B
#define SUCCESS_CODE 0x0C
#define FAILURE_CODE 0x0D
#define SHUTDOWN_CODE 0x0E

#define SERVER_QUEUE "/serverQueue"

#define MAX_MESSAGE_SIZE 1024
#define MAX_QUEUE_SIZE 10

#define MAX_SESSION_LINES 15

//TODO probably should be replaced by something more flexible
#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
            perror(#x); \
            exit(-1); \
        } \
    } while (0)  \

struct Message {
    char code;
    std::string data;
};

struct PipeEnds {
    int readEnd;
    int writeEnd;
};

void sendMessage(const Message& message, mqd_t mq);
void sendMessage(const Message& message, const std::string& queueName);
void sendCode(char code, mqd_t mq);
void sendCode(char code, const std::string& queueName);
Message receiveMessage(mqd_t mq);

mqd_t createMessageQueue(const std::string& name, int mode);
mqd_t getMessageQueue(const std::string& name, int mode);
void closeMessageQueue(mqd_t mq);
void deleteMessageQueue(const std::string& queueName);

std::string sessionInputQueueName(const std::string& sessionId);
std::string processOutputQueueName();

#endif //MY_SCREEN_COMMON_H
