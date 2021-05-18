#include <unistd.h>
#include <cassert>
#include <csignal>

#include "Client.h"
#include "Server.h"

Client::Client() {
    serverQueue = getMessageQueue(SERVER_QUEUE, O_WRONLY, false);
    if (serverQueue == static_cast<mqd_t>(-1)) {
        Server::spawn();
        int duration = 1000;
        while (true) {
            serverQueue = getMessageQueue(SERVER_QUEUE, O_WRONLY, false);
            if (serverQueue != static_cast<mqd_t>(-1)) break;
            usleep(duration);
            duration *= 2;
        }
    }

    outputQueueName = processOutputQueueName();
    outputQueue = createMessageQueue(outputQueueName, O_RDONLY);
}

Client::~Client() {
    // do not destroy anything in forked process
    if (inputPid != 0) {
        closeMessageQueue(serverQueue);
        if (inputQueue != 0) {
            closeMessageQueue(inputQueue);
        }
        closeMessageQueue(outputQueue);
        deleteMessageQueue(outputQueueName);
    }
}

std::string Client::newSession(const std::string& id) {
    sendMessage({NEW_SESSION_CODE, outputQueueName + " " + id}, serverQueue);

    Message message = receiveMessage(outputQueue);
    assert(message.code == SERVER_HELLO_CODE || message.code == FAILURE_CODE);
    if (message.code == SERVER_HELLO_CODE) {
        if (!id.empty()) {
            assert(id == message.data);
        }
        return message.data;
    } else {
        std::cout << message.data << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Client::attach(const std::string& newSessionId) {
    sendMessage({ATTACH_CODE, outputQueueName + " " + newSessionId}, serverQueue);

    Message message = receiveMessage(outputQueue);
    assert(message.code == SERVER_HELLO_CODE || message.code == FAILURE_CODE);
    if (message.code == SERVER_HELLO_CODE) {
        std::cout << "[" << newSessionId << "]" << std::endl;
        sessionId = newSessionId;
        assert(message.data == sessionId);
        inputQueue = getMessageQueue(sessionInputQueueName(sessionId), O_WRONLY);
    } else {
        std::cout << message.data << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Client::sendInput(const std::string &msg) const {
    sendMessage({STDIN_CODE, msg}, inputQueue);
}

void Client::detach() const {
    sendCode(DETACH_CODE, inputQueue);
}

void Client::kill(const std::string& id) {
    sendMessage({KILL_CODE, outputQueueName + " " + id}, serverQueue);

    auto message = receiveMessage(outputQueue);
    assert(message.code == SUCCESS_CODE || message.code == FAILURE_CODE);
    if (message.code == SUCCESS_CODE) {
        std::cout << "Killed session " << id << std::endl;
    } else {
        std::cout << message.data << std::endl;
    }
}

void Client::list() {
    sendMessage({LIST_CODE, outputQueueName}, serverQueue);
    auto message = receiveMessage(outputQueue);
    assert(message.code == SESSIONS_CODE);
    std::cout << message.data;
}

void Client::shutdown() {
    sendMessage({SHUTDOWN_CODE, outputQueueName}, serverQueue);
}

void Client::acceptMessages() const {
    while (true) {
        Message message = receiveMessage(outputQueue);

        switch (message.code) {
            case STDOUT_CODE:
                std::cout << message.data;
                std::flush(std::cout);
                break;
            case DETACHED_CODE:
                return;
            case TERMINATED_CODE:
                std::cout << message.data << std::endl;
                ::kill(inputPid, SIGINT);
                return;
            default:
                break;
        }
    }
}

void Client::handleInput() const {
    std::string input;
    while (true) {
        std::getline(std::cin, input);

        if (std::cin.eof()) {
            break;
        }

        input += '\n';
        sendInput(input);
    }
    detach();
}

void Client::start() {
    auto child = fork();
    if (child == 0) {
        handleInput();
        return;
    }
    inputPid = child;
    acceptMessages();
}
