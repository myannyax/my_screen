//
// Created by Maria.Filipanova on 5/5/21.
//

#ifndef MY_SCREEN_COMMON_H
#define MY_SCREEN_COMMON_H

//TODO need messages to pass info from server that some sessions have exited
#define NEW_SESSION 0x00
#define ATTACH 0x01
#define SERVER_HELLO 0x02
#define STDIN 0x03
#define STDOUT 0x04
#define STDERR 0x05
#define DETACH 0x06
#define KILL 0x07
#define LIST 0x08
#define SESSIONS 0x09
#define QUEUE_NAME "kek"

#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
            perror(#x); \
            exit(-1); \
        } \
    } while (0) \

#endif //MY_SCREEN_COMMON_H
