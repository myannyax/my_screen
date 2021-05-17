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

    exit(0);
}

void Server::attachClientToSession(const std::string& sessionId,
                           const std::string& outputQueueName) {
    sendMessage({ATTACH_CODE, outputQueueName}, sessionQueues[sessionId]);
}

void Server::killSession(const std::string& sessionId, const std::string& outputQueueName) {
    logic.killSession(sessionId);
    sendCode(KILL_CODE, sessionQueues[sessionId]);
    sessionQueues.erase(sessionId);

    auto outputQueue = getMessageQueue(outputQueueName, O_WRONLY);
    sendCode(KILLED_CODE, outputQueue);
    closeMessageQueue(outputQueue);
}

void Server::listSessions(const std::vector<std::string> &ids, const std::string& outputQueueName) {
    std::string result;

    for (const auto& id : ids) {
        result += id;
        result += '\n';
    }

    auto outputQueue = getMessageQueue(outputQueueName, O_WRONLY);
    sendMessage({SESSIONS_CODE, result}, outputQueue);
    closeMessageQueue(outputQueue);
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
            case ATTACH_CODE:
                std::cout << "attach received" << std::endl;
                {
                    std::istringstream is{message.data};
                    std::string id;
                    std::string outputQueueName;
                    is >> outputQueueName >> id;

                    attachClientToSession(id, outputQueueName);
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
    CHECK((mqd_t) - 1 != mq_close(inputQueue));
    //CHECK((mqd_t) - 1 != mq_close(mq_from));
    //CHECK((mqd_t) - 1 != mq_close(internal_mq));
}
