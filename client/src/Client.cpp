//
// Created by Maria.Filipanova on 5/5/21.
//

#include <unistd.h>
#include <cassert>

#include "Client.h"

Client::Client() {
    serverQueue = getMessageQueue(SERVER_QUEUE, O_WRONLY);

    outputQueueName = processOutputQueueName();
    outputQueue = createMessageQueue(outputQueueName, O_RDONLY);
}

Client::~Client() {
    closeMessageQueue(serverQueue);
    if (inputQueue != 0) {
        closeMessageQueue(inputQueue);
    }
    closeMessageQueue(outputQueue);
    deleteMessageQueue(outputQueueName);
}

std::string Client::newSession(const std::string& id) {
    sendMessage({NEW_SESSION_CODE, outputQueueName + " " + id}, serverQueue);

    Message message = receiveMessage(outputQueue);
    assert(message.code == SERVER_HELLO_CODE);
    if (!id.empty()) {
        assert(id == message.data);
    }

    return message.data;
}

void Client::attach(const std::string& newSessionId) {
    sessionId = newSessionId;
    sendMessage({ATTACH_CODE, outputQueueName + " " + sessionId}, serverQueue);

    Message message = receiveMessage(outputQueue);
    assert(message.code == SERVER_HELLO_CODE);
    sessionId = message.data;

    inputQueue = getMessageQueue(sessionInputQueueName(sessionId), O_WRONLY);
}

void Client::sendInput(const std::string &msg) {
    sendMessage({STDIN_CODE, msg}, inputQueue);
}

void Client::detach() {
    sendCode(DETACH_CODE, inputQueue);
}

void Client::kill(const std::string &sessionId) {
    sendMessage({KILL_CODE, outputQueueName + " " + sessionId}, serverQueue);

    auto message = receiveMessage(outputQueue);
    assert(message.code == KILLED_CODE);

    std::cout << "Killed session " << sessionId << std::endl;
}

void Client::list() {
    sendMessage({LIST_CODE, outputQueueName}, serverQueue);
    auto message = receiveMessage(outputQueue);
    assert(message.code == SESSIONS_CODE);
    std::cout << message.data;
}

void Client::acceptMessages() {
    while (true) {
        Message message = receiveMessage(outputQueue);

        switch (message.code) {
            case STDOUT_CODE:
                std::cout << message.data;
                break;
            case DETACHED_CODE:
            case TERMINATED_CODE:
                return;
            default:
                break;
        }
    }
}

void Client::handleInput() {
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
    inputWorker = std::thread{[&]() {
        handleInput();
    }};

    outputWorker = std::thread{[&]() {
        acceptMessages();
    }};
}

void Client::wait() {
    inputWorker.join();
    outputWorker.join();
}
