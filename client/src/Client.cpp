//
// Created by Maria.Filipanova on 5/5/21.
//

#include <unistd.h>
#include <cassert>
#include <csignal>

#include "Client.h"

Client::Client() {
    serverQueue = getMessageQueue(SERVER_QUEUE, O_WRONLY);

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
    assert(message.code == SERVER_HELLO_CODE);
    if (!id.empty()) {
        assert(id == message.data);
    }

    return message.data;
}

void Client::attach(const std::string& newSessionId) {
    sessionId = newSessionId;

    inputQueue = getMessageQueue(sessionInputQueueName(sessionId), O_WRONLY);
    sendMessage({ATTACH_CODE, outputQueueName}, inputQueue);

    Message message = receiveMessage(outputQueue);
    assert(message.code == SERVER_HELLO_CODE);
    assert(message.data == sessionId);
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
    assert(message.code == SUCCESS_CODE || message.code == FAILURE_CODE);
    if (message.code == SUCCESS_CODE) {
        std::cout << "Killed session " << sessionId << std::endl;
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

void Client::acceptMessages() {
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
    auto child = fork();
    if (child == 0) {
        handleInput();
        return;
    }
    inputPid = child;
    acceptMessages();
}
