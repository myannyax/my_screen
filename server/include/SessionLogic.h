#ifndef MY_SCREEN_SESSIONLOGIC_H
#define MY_SCREEN_SESSIONLOGIC_H

#include <deque>
#include <string>
#include <vector>

class SessionLogic {
private:
    std::deque<std::string> buffer;

public:
    std::vector<std::string> getSessionBuffer();

    void receiveInput(const std::string& data);

    void sendOutput(const std::string& data);

private:
    void addData(const std::string& data);
};

#endif //MY_SCREEN_SESSIONLOGIC_H
