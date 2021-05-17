//
// Created by Maria.Filipanova on 5/5/21.
//

#include <sstream>
#include <unistd.h>
#include <Session.h>

#include "Server.h"

Server::Server() {
    inputQueue = createMessageQueue(SERVER_QUEUE, O_RDONLY);
}

void Server::createNewSession(const std::string& id, const std::string& outputQueueName) {
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

void Server::killSession(const std::string& sessionId, const std::string& outputQueueName) {
    if (!sessionQueues.contains(sessionId)) {
        sendMessage({FAILURE_CODE, "No session with id=" + sessionId}, outputQueueName);
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

        switch (message.code) {
            case NEW_SESSION_CODE:
                std::cout << "newSession received" << std::endl;
                {
                    std::istringstream is{message.data};
                    std::string id;
                    std::string outputQueueName;
                    is >> outputQueueName >> id;

                    createNewSession(id, outputQueueName);
                }
                break;
            case KILL_CODE:
                std::cout << "kill received" << std::endl;
                {
                    std::istringstream is{message.data};
                    std::string id;
                    std::string outputQueueName;
                    is >> outputQueueName >> id;

                    killSession(id, outputQueueName);
                }
                break;
            case LIST_CODE:
                std::cout << "list received" << std::endl;
                listSessions(logic.getSessionIds(), message.data);
                break;
            default :
                std::cout << "kek received" << std::endl;
                break;
        }
    }
}

Server::~Server() {
    closeMessageQueue(inputQueue);
}
