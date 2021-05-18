#include <sstream>

#include "Common.h"
#include "SessionLogic.h"

std::vector<std::string> SessionLogic::getSessionBuffer() {
    return std::vector<std::string>{buffer.begin(), buffer.end()};
}

void SessionLogic::receiveInput(const std::string& data) {
    addData(data);
}

void SessionLogic::sendOutput(const std::string& data) {
    addData(data);
}

void SessionLogic::addData(const std::string& data) {
    if (data.empty()) {
        return;
    }

    std::string rawData;
    if (!buffer.empty() && buffer.back().back() != '\n') {
        rawData += buffer.back();
        buffer.pop_back();
    }
    rawData += data;

    std::istringstream is{rawData};
    std::string line;
    while (std::getline(is, line)) {
        buffer.push_back(line + '\n');
    }
    if (data.back() != '\n') {
        buffer.back().pop_back();
    }

    while (buffer.size() > MAX_SESSION_LINES) {
        buffer.pop_front();
    }
}
