#include <unistd.h>
#include <iostream>
#include <sstream>
#include <utility>

#include "Common.h"
#include "Session.h"

Session::Session(const std::string& sessionId, const std::string& outputQueueName) : sessionId(sessionId) {
    inputQueueName = sessionInputQueueName(sessionId);
    inputQueue = getMessageQueue(inputQueueName, O_RDONLY);

    outputQueue = getMessageQueue(outputQueueName, O_WRONLY);

    CHECK(pipe2(reinterpret_cast<int(&)[2]>(stdinEnds), O_CLOEXEC) == 0);
    CHECK(pipe2(reinterpret_cast<int(&)[2]>(stdoutEnds), O_CLOEXEC) == 0);
    dup2(stdinEnds.readEnd, STDIN_FILENO);
    dup2(stdoutEnds.writeEnd, STDOUT_FILENO);
    dup2(stdoutEnds.writeEnd, STDERR_FILENO);
}

Session::~Session() {
}

void Session::start() {
    shellWorker = std::thread{[]() {
        try {
            std::system("bash");
        } catch (const std::exception& e) {
            // TODO
        }
    }};

    inputWorker = std::thread{[&]() {
        acceptMessages();
    }};

    outputWorker = std::thread{[&]() {
        while (processOutput()) {}

        terminateSession();
    }};
}

void Session::wait() {
    inputWorker.join();
    outputWorker.join();
    shellWorker.join();
}

void Session::attachClient(const std::string& outputQueueName) {
    outputQueue = getMessageQueue(outputQueueName, O_WRONLY);

    activeSessionMutex.lock();
    activeSession = true;
    sendMessage({SERVER_HELLO_CODE, sessionId}, outputQueue);
    for (const auto& msg : logic.getSessionBuffer()) {
        sendMessage({STDOUT_CODE, msg}, outputQueue);
    }
    activeSessionMutex.unlock();
}

void Session::detachClient() {
    activeSessionMutex.lock();
    activeSession = false;
    sendCode(DETACHED_CODE, outputQueue);
    closeMessageQueue(outputQueue);
    outputQueue = 0;
    activeSessionMutex.unlock();
}

void Session::receiveInput(const std::string &msg) {
    activeSessionMutex.lock();
    logic.receiveInput(msg);
    activeSessionMutex.unlock();

    write(stdinEnds.writeEnd, msg.c_str(), msg.size());
}

bool Session::processOutput() {
    char buffer[MAX_MESSAGE_SIZE];

    ssize_t bytes_read = read(stdoutEnds.readEnd, buffer, MAX_MESSAGE_SIZE);
    if (bytes_read <= 0) {
        std::cerr << "AAAAAAABBBB" << std::endl;
        return false;
    }

    auto msg = std::string{buffer, static_cast<std::size_t>(bytes_read)};

    activeSessionMutex.lock();
    logic.sendOutput(msg);
    if (activeSession) {
        sendMessage({STDOUT_CODE, msg}, outputQueue);
    }
    activeSessionMutex.unlock();

    return true;
}

void Session::terminateSession() {
    activeSessionMutex.lock();
    if (activeSession) {
        sendCode(TERMINATED_CODE, outputQueue);
    }
    activeSessionMutex.unlock();

    // TODO
}

void Session::acceptMessages() {
    while (true) {
        Message message = receiveMessage(inputQueue);

        switch (message.code) {
            case ATTACH_CODE:
                attachClient(message.data);
                break;
            case STDIN_CODE:
                receiveInput(message.data);
                break;
            case DETACH_CODE:
                detachClient();
                break;
            case KILL_CODE:
                terminateSession();
                return;
            default:
                break;
        }
    }
}
