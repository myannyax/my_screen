//
// Created by Maria.Filipanova on 5/5/21.
//

#include <sstream>
#include <unistd.h>
#include <Session.h>

#include "Server.h"

namespace {
    void parseMessage(const std::string& data, std::string& sessionId, std::string& outputQueueName) {
        std::istringstream is{data};
        is >> outputQueueName >> sessionId;
    }
}

Server::Server() {
    inputQueue = createMessageQueue(SERVER_QUEUE, O_RDONLY);
}

void Server::createNewSession(const std::string& id, const std::string& outputQueueName) {
    if (sessionQueues.contains(id)) {
        sendMessage({FAILURE_CODE, "Session " + id + " already exists"}, outputQueueName);
        return;
    }

    auto sessionId = logic.createNewSession(id);
    auto inputQueueName = sessionInputQueueName(sessionId);
    sessionQueues[sessionId] = createMessageQueue(inputQueueName, O_WRONLY);

    sendMessage({SERVER_HELLO_CODE, sessionId}, getMessageQueue(outputQueueName, O_WRONLY));

    auto child = fork();
    if (child > 0) {
        return;
    }

    Session session{sessionId, inputQueueName};
    session.start();

    session.wait();
    exit(EXIT_SUCCESS);
}

void Server::attachClientToSession(const std::string& sessionId,
                                   const std::string& outputQueueName) {
    if (!sessionQueues.contains(sessionId)) {
        sendMessage({FAILURE_CODE, "Session " + sessionId + " does not exist"}, outputQueueName);
        return;
    }
    sendMessage({ATTACH_CODE, outputQueueName}, sessionQueues[sessionId]);
}

void Server::killSession(const std::string& sessionId, const std::string& outputQueueName) {
    if (!sessionQueues.contains(sessionId)) {
        sendMessage({FAILURE_CODE, "Session " + sessionId + " does not exist"}, outputQueueName);
        return;
    }
    logic.killSession(sessionId);
    sendCode(KILL_CODE, sessionQueues[sessionId]);
    deleteMessageQueue(sessionInputQueueName(sessionId));
    sessionQueues.erase(sessionId);

    sendCode(SUCCESS_CODE, outputQueueName);
}

void Server::listSessions(const std::vector<std::string> &ids, const std::string& outputQueueName) {
    std::string result;

    for (const auto& id : ids) {
        result += id;
        result += '\n';
    }
    sendMessage({SESSIONS_CODE, result}, outputQueueName);
}

void Server::acceptMessages() {
    while (true) {
        Message message = receiveMessage(inputQueue);

        std::string id;
        std::string outputQueueName;

        switch (message.code) {
            case NEW_SESSION_CODE:
                std::cout << "newSession received" << std::endl;
                parseMessage(message.data, id, outputQueueName);
                createNewSession(id, outputQueueName);
                break;
            case ATTACH_CODE:
                std::cout << "attach received" << std::endl;
                parseMessage(message.data, id, outputQueueName);
                attachClientToSession(id, outputQueueName);
                break;
            case KILL_CODE:
                parseMessage(message.data, id, outputQueueName);
                killSession(id, outputQueueName);
                break;
            case LIST_CODE:
                std::cout << "list received" << std::endl;
                listSessions(logic.getSessionIds(), message.data);
                break;
            case SHUTDOWN_CODE:
                std::cout << "shutdown received" << std::endl;
                return;
            default:
                std::cout << "unrecognized code: " << message.code << std::endl;
                break;
        }
    }
}

Server::~Server() {
    std::string tmpOutputQueueName = "/tmpOutputQueue";
    auto tmpOutputQueue = createMessageQueue(tmpOutputQueueName, O_WRONLY);

    for (const auto& id : logic.getSessionIds()) {
        killSession(id, tmpOutputQueueName);
    }

    closeMessageQueue(tmpOutputQueue);
    deleteMessageQueue(tmpOutputQueueName);

    closeMessageQueue(inputQueue);
    deleteMessageQueue(SERVER_QUEUE);
}
