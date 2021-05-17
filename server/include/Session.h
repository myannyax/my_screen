#ifndef MY_SCREEN_SESSION_H
#define MY_SCREEN_SESSION_H

#include <condition_variable>
#include <mqueue.h>
#include <string>
#include <vector>
#include <mutex>
#include <thread>

#include "Common.h"
#include "SessionLogic.h"

class Session {
public:
    Session(const std::string &sessionId, const std::string &outputQueueName);

    ~Session();

    void start();

    void wait();

    void attachClient(const std::string& outputQueueName);

    void detachClient();

    void receiveInput(const std::string &msg);

    bool processOutput();

    void terminateSession();

    void acceptMessages();

private:
    std::string sessionId;

    SessionLogic logic;

    std::mutex activeSessionMutex;
    bool activeSession = false;

    std::string inputQueueName;

    mqd_t inputQueue;
    mqd_t outputQueue = 0;

    PipeEnds stdinEnds{};
    PipeEnds stdoutEnds{};

    std::thread shellWorker;
    std::thread inputWorker;
    std::thread outputWorker;
};

#endif //MY_SCREEN_SESSION_H
