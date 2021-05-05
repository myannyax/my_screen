//
// Created by Maria.Filipanova on 5/5/21.
//

#include "Common.h"

void sendString(char code, const std::string &msg, mqd_t mq) {
    char buffer[1024];
    buffer[0] = code;
    char* my_buff_ptr = buffer + 1;
    const char* my_str_end = msg.c_str() + msg.size();
    const char* my_str_ptr = msg.c_str() + msg.size();
    while(my_str_end > my_str_ptr) {
        size_t size = std::max<size_t>(my_str_end - my_str_ptr, MAX_SIZE);
        memcpy(my_buff_ptr, my_str_ptr, size);
        CHECK(0 <= mq_send(mq, buffer, size, 0));
        my_str_ptr += size;
        my_buff_ptr += size;
    }
}
