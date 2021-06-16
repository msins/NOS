#include <chrono>
#include <unistd.h>
#include <csignal>
#include <thread>
#include <deque>
#include "queues.h"
#include "logging.h"
#include "random.h"

typedef std::chrono::milliseconds ms;

auto lastChange = std::chrono::system_clock::now();
int timeout;

void resetTimer() {
    lastChange = std::chrono::system_clock::now();
}

bool timeoutExpired() {
    auto now = std::chrono::system_clock::now();
    ms passed = std::chrono::duration_cast<ms>(now - lastChange);
    if (passed.count() >= timeout) {
        lastChange = now;
        return true;
    }

    return false;
}

int carQueueId;
int commandQueueId;

void clear(int failure) {
    clearQueue(carQueueId);
    clearQueue(commandQueueId);
    log("Cleared queues.");
    exit(failure);
}

int main(int agrc, char *argv[]) {
    key_t carQueueKey = CAR_QUEUE_KEY;
    key_t commandQueueKey = COMMAND_QUEUE_KEY;

    int numOfCars = std::stoi(argv[1]);
    int carsQueuedInDirection = 0;
    int direction = randomInt(0, 1);
    timeout = randomInt(500, 1000);

    signal(SIGTERM, clear);
    signal(SIGINT, clear);

    carQueueId = connectToQueue(carQueueKey);
    commandQueueId = connectToQueue(commandQueueKey);

    std::deque<CarMessage> queue;
    log("direction: " + std::to_string(direction));
    while (numOfCars > 0) {
        while (!timeoutExpired() && carsQueuedInDirection < 3) {
            // read, if possible, next message in message queue
            CarMessage message{};

            if (receiveFromCarQueue(carQueueId, &message, direction) == -1) {
                if (errno == ENOMSG) {
                    continue;
                }
                perror("Failed to read from message queue");
                exit(1);
            }

            queue.push_front(message);
            carsQueuedInDirection++;
        }

        while (!queue.empty()) {
            CarMessage message = queue.front();
            queue.pop_front();

            CommandMessage command{};
            command.car = message.car;
            // add car registration as message type so car process can pop it with it's own registration
            command.type = message.car.registration;

            strcpy(command.state, PASSING_STATE);
            addToCommandQueue(commandQueueId, &command);

            // car is passing over the bridge
            int timeToCross = randomInt(1000, 3000);
            std::this_thread::sleep_for(std::chrono::milliseconds(timeToCross));

            strcpy(command.state, PASSED_STATE);
            addToCommandQueue(commandQueueId, &command);

            numOfCars--;
        }

        carsQueuedInDirection = 0;
        direction ^= 1;
        resetTimer();
        log("direction: " + std::to_string(direction));
    }

    sleep(1);
    clear(0);
    return 0;
}
