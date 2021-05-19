#include <unistd.h>
#include <utility>
#include <sys/wait.h>
#include <pty.h>

#include "Common.h"
#include "Session.h"

Session::Session(const std::string& sessionId, const std::string& outputQueueName) : sessionId(sessionId) {
    inputQueueName = sessionInputQueueName(sessionId);
    inputQueue = getMessageQueue(inputQueueName, O_RDWR);

    outputQueue = getMessageQueue(outputQueueName, O_WRONLY);

    CHECK(openpty(&amaster, &aslave, nullptr, nullptr, nullptr) >= 0);
}

Session::~Session() {
    closeMessageQueue(inputQueue);
    closeMessageQueue(outputQueue);
}

void Session::start() {
    auto child = fork();
    CHECK(child >= 0);

    if (child == 0) {
        CHECK(close(amaster) == 0);
        dup2(aslave, STDIN_FILENO);
        dup2(aslave, STDOUT_FILENO);
        dup2(aslave, STDERR_FILENO);

        struct termios tp;
        CHECK(tcgetattr(STDIN_FILENO, &tp) != -1);
        tp.c_lflag &= ~ECHO;
        CHECK(tcsetattr(STDIN_FILENO, TCSAFLUSH, &tp) != -1);

        execl("/bin/bash", "/bin/bash", nullptr);
        perror("execve");
        exit(EXIT_FAILURE);
    }

    CHECK(close(aslave) == 0);
    shellPid = child;

    inputWorker = std::thread{[&]() {
        acceptMessages();
    }};

    outputWorker = std::thread{[&]() {
        while (processOutput()) {}
        // std::cout << "session " << sessionId << ": stop processing output" << std::endl;
    }};
}

void Session::wait() {
    int status;
    CHECK(waitpid(shellPid, &status, 0) == shellPid);

    endSession(status);

    inputWorker.join();
    outputWorker.join();
}

void Session::attachClient(const std::string& outputQueueName) {
    auto newOutputQueue = getMessageQueue(outputQueueName, O_WRONLY);

    activeSessionMutex.lock();
    if (activeSession) {
        sendMessage({FAILURE_CODE, "No more than one client can be attached to session"}, newOutputQueue);
        closeMessageQueue(newOutputQueue);
        activeSessionMutex.unlock();
        return;
    }
    outputQueue = newOutputQueue;
    activeSession = true;
    sendMessage({SERVER_HELLO_CODE, sessionId}, outputQueue);
    for (const auto& msg : logic.getSessionBuffer()) {
        sendMessage({STDOUT_CODE, msg}, outputQueue);
    }
    activeSessionMutex.unlock();
}

void Session::detachClient() {
    activeSessionMutex.lock();
    activeSession = false;
    sendCode(DETACHED_CODE, outputQueue);
    closeMessageQueue(outputQueue);
    outputQueue = 0;
    activeSessionMutex.unlock();
}

void Session::receiveInput(const std::string &msg) {
    activeSessionMutex.lock();
    logic.receiveInput(msg);
    activeSessionMutex.unlock();

    write(amaster, msg.data(), msg.size());
}

bool Session::processOutput() {
    char buffer[MAX_MESSAGE_SIZE];

    ssize_t bytes_read = read(amaster, buffer, MAX_MESSAGE_SIZE);
    if (bytes_read <= 0) {
        return false;
    }

    auto msg = std::string{buffer, static_cast<std::size_t>(bytes_read)};

    activeSessionMutex.lock();
    logic.sendOutput(msg);
    if (activeSession) {
        sendMessage({STDOUT_CODE, msg}, outputQueue);
    }
    activeSessionMutex.unlock();

    return true;
}

void Session::killSession() const {
    kill(shellPid, SIGKILL);
}

void Session::endSession(int status) {
    CHECK(close(amaster) == 0);

    sendCode(TERMINATED_CODE, inputQueue);
    sendMessage({TERMINATED_CODE, sessionId}, SERVER_QUEUE);

    activeSessionMutex.lock();
    if (activeSession) {
        std::string message;
        if (WIFEXITED(status)) {
            message = "Session " + sessionId + " exited with code " + std::to_string(WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            message = "Session " + sessionId + " terminated by signal " + std::to_string(WTERMSIG(status));
        } else if (WCOREDUMP(status)) {
            message = "Session " + sessionId + " aborted: core dumped";
        }
        sendMessage({TERMINATED_CODE, message}, outputQueue);
    }
    activeSessionMutex.unlock();
}

void Session::acceptMessages() {
    while (true) {
        Message message = receiveMessage(inputQueue);

        switch (message.code) {
            case ATTACH_CODE:
                // std::cout << "session " << sessionId << ": attach received" << std::endl;
                attachClient(message.data);
                break;
            case STDIN_CODE:
                // std::cout << "session " << sessionId << ": stdin received" << std::endl;
                receiveInput(message.data);
                break;
            case DETACH_CODE:
                // std::cout << "session " << sessionId << ": detach received" << std::endl;
                detachClient();
                break;
            case KILL_CODE:
                // std::cout << "session " << sessionId << ": kill received" << std::endl;
                killSession();

                // std::cout << "session " << sessionId << ": stop processing input" << std::endl;
                return;
            case TERMINATED_CODE:
                // std::cout << "session " << sessionId << ": terminated" << std::endl;
                // std::cout << "session " << sessionId << ": stop processing input" << std::endl;
                return;
            default:
                break;
        }
    }
}
