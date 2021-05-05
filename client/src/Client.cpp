//
// Created by Maria.Filipanova on 5/5/21.
//

#include "Client.h"

Client::Client() {
    mq_to = mq_open(QUEUE_NAME_SC, O_RDONLY);
    mq_from = mq_open(QUEUE_NAME_CS, O_WRONLY);
    CHECK((mqd_t) - 1 != mq_to);
    CHECK((mqd_t) - 1 != mq_from);
}

void Client::newSession(std::string sessionId = "") {
    //TODO assert id size
    sendString(NEW_SESSION, sessionId, mq_from);
}

void Client::attach(std::string sessionId) {
    //TODO assert id size
    sendString(ATTACH, sessionId, mq_from);
}

void Client::sendSTDIN(const std::string &msg) {
    sendString(STDIN, msg, mq_from);
}

void Client::detach() {
    constexpr unsigned int size = 1;
    char buffer[size];
    buffer[0] = DETACH;
    CHECK(0 <= mq_send(mq_from, buffer, size, 0));
}

void Client::kill(std::string sessionId) {
    //TODO assert id size
    sendString(KILL, sessionId, mq_from);
}

void Client::list() {
    constexpr unsigned int size = 1;
    char buffer[size];
    buffer[0] = LIST;
    CHECK(0 <= mq_send(mq_from, buffer, size, 0));
}

void Client::acceptMessages() {
    char buffer[MAX_SIZE + 1];

    while (true) {
        ssize_t bytes_read;

        /* receive the message */
        bytes_read = mq_receive(mq_to, buffer, MAX_SIZE, NULL);
        CHECK(bytes_read >= 0);

        buffer[bytes_read] = '\0';
        std::string id;
        switch (buffer[0]) {
            case SERVER_HELLO:
                std::cout << std::string(buffer + 1) << std::endl;
                break;
            case STDOUT:
                std::cout << std::string(buffer + 1) << std::endl;
                break;
            case STDERR:
                std::cerr << std::string(buffer + 1) << std::endl;
                break;
            case SESSIONS:
                std::vector<std::string> res;
                char *my_buff_ptr = buffer + 1;
                while (*my_buff_ptr != '\0') {
                    auto str = std::string(my_buff_ptr);
                    my_buff_ptr += (str.size() + 1);
                    res.emplace_back(str);
                }
                for (const auto &kek: res) {
                    std::cout << kek << " ";
                }
                std::cout << std::endl;
                break;
        }
    }
}

Client::~Client() {
    CHECK((mqd_t) - 1 != mq_close(mq_to));
    CHECK((mqd_t) - 1 != mq_close(mq_from));
}
