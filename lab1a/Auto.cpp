#include <thread>
#include "logging.h"
#include "random.h"
#include "messages.h"
#include "queues.h"

int main(int argc, char *argv[]) {
    int registration = std::stoi(argv[1]);
    int direction = std::stoi(argv[2]);
    Car car = {registration, direction};

    key_t carQueueKey = CAR_QUEUE_KEY;
    key_t commandQueueKey = COMMAND_QUEUE_KEY;

    int timeToSleep = randomInt(100, 2000);
    std::this_thread::sleep_for(std::chrono::milliseconds(timeToSleep));

    // connect to a queue which sends requests to semaphore
    int carQueueId = connectToQueue(carQueueKey);

    // connect to a queue which receives commands from semaphore
    int commandQueueId = connectToQueue(commandQueueKey);

    // message type is direction of crossing so semaphore could decide which cars to let across
    CarMessage message{car.direction + 1, car};

    // send request for crossing to the semaphore
    addToCarQueue(carQueueId, &message);
    log("Automobil " + std::to_string(car.registration) + " ceka na prelazak preko mosta.");

    // receive response from the semaphore, message type is registration which is unique
    CommandMessage command{};
    receiveFromCommandQueue(commandQueueId, &command, car.registration);
    log("Automobil " + std::to_string(command.car.registration) + " se popeo na most.");

    receiveFromCommandQueue(commandQueueId, &command, car.registration);
    log("Automobil " + std::to_string(command.car.registration) + " je presao most.");

    return 0;
}

