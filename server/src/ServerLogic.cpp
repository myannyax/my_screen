#include "ServerLogic.h"

std::string ServerLogic::createNewSession(std::string sessionId) {
    if (sessionId.empty()) {
        sessionId = sessionIds.generateId();
    } else {
        sessionIds.insert(sessionId);
    }

    // std::cout << "new session created" << std::endl;
    return sessionId;
}

void ServerLogic::endSession(const std::string& sessionId) {
    sessionIds.remove(sessionId);
}

std::vector<std::string> ServerLogic::getSessionIds() {
    std::vector<std::string> result{sessionIds.get_ids().begin(), sessionIds.get_ids().end()};
    std::sort(result.begin(), result.end());
    return result;
}

ServerLogic::IdContainer::IdContainer(std::string prefix) : prefix(std::move(prefix)) {}

bool ServerLogic::IdContainer::insert(const std::string& id) {
    if (ids.contains(id)) {
        return false;
    }
    ids.insert(id);
    return true;
}

bool ServerLogic::IdContainer::remove(const std::string& id) {
    if (!ids.contains(id)) {
        return false;
    }
    ids.erase(id);
    return true;
}

std::string ServerLogic::IdContainer::generateId() {
    std::string new_id;
    do {
        new_id = prefix + std::to_string(counter++);
    } while (ids.contains(new_id));

    ids.insert(new_id);
    return new_id;
}

const std::unordered_set<std::string>& ServerLogic::IdContainer::get_ids() {
    return ids;
}
