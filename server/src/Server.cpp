#include <sstream>
#include <unistd.h>
#include <regex>
#include <csignal>

#include "Server.h"
#include "Session.h"

namespace {
    void serverSignalHandler(int) {
    }
}

void parseMessage(const std::string& data, std::string& sessionId, std::string& outputQueueName) {
    std::istringstream is{data};
    is >> outputQueueName;
    std::getline(is >> std::ws, sessionId, {});
}

Server::Server() {
    inputQueue = createMessageQueue(SERVER_QUEUE, O_RDONLY);
    signal(SIGINT, serverSignalHandler);
    signal(SIGHUP, serverSignalHandler);
}

void Server::spawn() {
    auto spawner = fork();
    CHECK(spawner >= 0);

    if (spawner == 0) {
        auto daemon = fork();
        CHECK(daemon >= 0);
        if (daemon > 0) {
            exit(0);
        }

        {
            Server server;
            server.acceptMessages();
        }

        exit(0);
    }
}

void Server::createNewSession(const std::string& id, const std::string& outputQueueName) {
    if (sessionQueues.contains(id)) {
        sendMessage({FAILURE_CODE, "Session " + id + " already exists"}, outputQueueName);
        return;
    }
    if (!std::regex_match(id, std::regex{SESSION_ID_FORMAT})) {
        sendMessage({FAILURE_CODE, "Incorrect session id: " + id}, outputQueueName);
        return;
    }

    auto sessionId = logic.createNewSession(id);
    auto inputQueueName = sessionInputQueueName(sessionId);
    sessionQueues[sessionId] = createMessageQueue(inputQueueName, O_WRONLY);

    sendMessage({SERVER_HELLO_CODE, sessionId}, outputQueueName);

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
    sendCode(KILL_CODE, sessionQueues[sessionId]);
    sendCode(SUCCESS_CODE, outputQueueName);
}

void Server::endSession(const std::string& sessionId) {
    logic.endSession(sessionId);
    deleteMessageQueue(sessionInputQueueName(sessionId));
    sessionQueues.erase(sessionId);
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
                // std::cout << "newSession received" << std::endl;
                parseMessage(message.data, id, outputQueueName);
                createNewSession(id, outputQueueName);
                break;
            case ATTACH_CODE:
                // std::cout << "attach received" << std::endl;
                parseMessage(message.data, id, outputQueueName);
                attachClientToSession(id, outputQueueName);
                break;
            case KILL_CODE:
                // std::cout << "kill received" << std::endl;
                parseMessage(message.data, id, outputQueueName);
                killSession(id, outputQueueName);
                break;
            case LIST_CODE:
                // std::cout << "list received" << std::endl;
                listSessions(logic.getSessionIds(), message.data);
                break;
            case TERMINATED_CODE:
                // std::cout << "terminated received" << std::endl;
                endSession(message.data);
                if (sessionQueues.empty()) {
                    return;
                }
                break;
            case SHUTDOWN_CODE:
                // std::cout << "shutdown received" << std::endl;
                return;
            default:
                // std::cout << "unrecognized code: " << message.code << std::endl;
                break;
        }
    }
}

Server::~Server() {
    std::string tmpOutputQueueName = "/my_screen_tmp_output_queue";
    auto tmpOutputQueue = createMessageQueue(tmpOutputQueueName, O_RDONLY);

    for (const auto& id : logic.getSessionIds()) {
        killSession(id, tmpOutputQueueName);

        auto message = receiveMessage(tmpOutputQueue);
        CHECK(message.code == SUCCESS_CODE);
        message = receiveMessage(inputQueue);
        CHECK(message.code == TERMINATED_CODE);

        endSession(id);
    }

    closeMessageQueue(tmpOutputQueue);
    deleteMessageQueue(tmpOutputQueueName);

    closeMessageQueue(inputQueue);
    deleteMessageQueue(SERVER_QUEUE);
}
