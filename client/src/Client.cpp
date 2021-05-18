#include <unistd.h>
#include <cassert>
#include <csignal>

#include "Client.h"
#include "Server.h"

namespace {
    std::string SessionId;

    void inputProcessSignalHandler(int) {
        if (!SessionId.empty()) {
            auto mq = getMessageQueue(sessionInputQueueName(SessionId), O_WRONLY, false);
            if (mq != -1) {
                sendCode(DETACH_CODE, mq);
                closeMessageQueue(mq);
            }
        }
        exit(0);
    }
    void clientSignalHandler(int) {
    }
}

Client::Client() {
    serverQueue = getMessageQueue(SERVER_QUEUE, O_WRONLY, false);

    outputQueueName = processOutputQueueName();
    outputQueue = createMessageQueue(outputQueueName, O_RDONLY);
}

Client::~Client() {
    // do not destroy anything in forked process
    if (!doNotDestroy) {
        if (serverExists()) {
            closeMessageQueue(serverQueue);
        }
        if (inputQueue != 0) {
            closeMessageQueue(inputQueue);
        }
        closeMessageQueue(outputQueue);
        deleteMessageQueue(outputQueueName);
    }
}

std::string Client::newSession(const std::string& id) {
    spawnServer();

    sendMessage({NEW_SESSION_CODE, outputQueueName + " " + id}, serverQueue);

    Message message = receiveMessage(outputQueue);
    assert(message.code == SERVER_HELLO_CODE || message.code == FAILURE_CODE);

    if (message.code == FAILURE_CODE) {
        std::cout << message.data << std::endl;
        return "";
    }
    if (!id.empty()) {
        assert(id == message.data);
    }
    return message.data;
}

bool Client::attach(const std::string& id) {
    if (!serverExists()) {
        std::cout << "Session " << id << " does not exist" << std::endl;
        return false;
    }

    sendMessage({ATTACH_CODE, outputQueueName + " " + id}, serverQueue);

    Message message = receiveMessage(outputQueue);
    assert(message.code == SERVER_HELLO_CODE || message.code == FAILURE_CODE);

    if (message.code == FAILURE_CODE) {
        std::cout << message.data << std::endl;
        return false;
    }

    std::cout << "[" << id << "]" << std::endl;
    sessionId = id;
    assert(message.data == sessionId);
    inputQueue = getMessageQueue(sessionInputQueueName(sessionId), O_WRONLY);

    return true;
}

void Client::detach() const {
    sendCode(DETACH_CODE, inputQueue);
}

void Client::kill(const std::string& id) {
    if (!serverExists()) {
        std::cout << "Session " << id << " does not exist" << std::endl;
        return;
    }

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
    if (!serverExists()) {
        return;
    }

    sendMessage({LIST_CODE, outputQueueName}, serverQueue);
    auto message = receiveMessage(outputQueue);
    assert(message.code == SESSIONS_CODE);
    std::cout << message.data;
}

void Client::shutdown() {
    sendMessage({SHUTDOWN_CODE, outputQueueName}, serverQueue);
}

void Client::start() {
    auto child = fork();
    if (child == 0) {
        doNotDestroy = true;
        SessionId = sessionId;
        signal(SIGINT, inputProcessSignalHandler);
        handleInput();
        return;
    }
    signal(SIGINT, clientSignalHandler);
    inputPid = child;
    acceptMessages();
}

void Client::sendInput(const std::string &msg) const {
    sendMessage({STDIN_CODE, msg}, inputQueue);
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

bool Client::serverExists() const {
    return serverQueue != -1;
}

void Client::spawnServer() {
    if (!serverExists()) {
        Server::spawn();
        int duration = 1000;
        while (true) {
            serverQueue = getMessageQueue(SERVER_QUEUE, O_WRONLY, false);
            if (serverExists())
                break;
            usleep(duration);
            duration *= 2;
        }
    }
}
