#include <unistd.h>

#include "Common.h"

void sendMessage(const Message& message, mqd_t mq) {
    if (message.data.empty()) {
        sendCode(message.code, mq);
    }

    char buffer[MAX_MESSAGE_SIZE];

    buffer[0] = message.code;
    char *my_buff_ptr = buffer + 1;
    const char *my_str_end = message.data.c_str() + message.data.size();
    const char *my_str_ptr = message.data.c_str();
    while (my_str_end > my_str_ptr) {
        size_t size = std::min<size_t>(my_str_end - my_str_ptr, MAX_MESSAGE_SIZE - 1);
        memcpy(my_buff_ptr, my_str_ptr, size);
        CHECK(mq_send(mq, buffer, size + 1, 0) == 0);
        my_str_ptr += size;
        my_buff_ptr += size;
    }
}

void sendMessage(const Message& message, const std::string& queueName) {
    auto mq = getMessageQueue(queueName, O_WRONLY);
    sendMessage(message, mq);
    closeMessageQueue(mq);
}

void sendCode(char code, mqd_t mq) {
    CHECK(mq_send(mq, &code, 1, 0) == 0);
}

void sendCode(char code, const std::string& queueName) {
    auto mq = getMessageQueue(queueName, O_WRONLY);
    sendCode(code, mq);
    closeMessageQueue(mq);
}

Message receiveMessage(mqd_t mq) {
    char buffer[MAX_MESSAGE_SIZE];
    auto bytes_read = mq_receive(mq, buffer, MAX_MESSAGE_SIZE, nullptr);
    CHECK(bytes_read >= 0);

    Message message;
    message.code = buffer[0];
    message.data = std::string{buffer + 1, static_cast<std::size_t>(bytes_read - 1)};

    return message;
}

mqd_t createMessageQueue(const std::string& name, int mode) {
    struct mq_attr attr{};

    /* initialize the queue attributes */
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_QUEUE_SIZE;
    attr.mq_msgsize = MAX_MESSAGE_SIZE;
    attr.mq_curmsgs = 0;

    auto mq = mq_open(name.c_str(), O_CREAT | mode, 0644, &attr);
    CHECK(mq != static_cast<mqd_t>(-1));

    return mq;
}

mqd_t getMessageQueue(const std::string& name, int mode) {
    auto mq = mq_open(name.c_str(), mode);
    CHECK(mq != static_cast<mqd_t>(-1));

    return mq;
}

void closeMessageQueue(mqd_t mq) {
    CHECK(mq_close(mq) != static_cast<mqd_t>(-1));
}

void deleteMessageQueue(const std::string& queueName) {
    CHECK(mq_unlink(queueName.c_str()) != static_cast<mqd_t>(-1));
}

std::string sessionInputQueueName(const std::string& sessionId) {
    return "/in_" + sessionId;
}

std::string processOutputQueueName() {
    return "/out_" + std::to_string(getpid());
}
