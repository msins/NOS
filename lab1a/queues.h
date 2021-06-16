#ifndef LAB1A_QUEUES_H
#define LAB1A_QUEUES_H

#include <sys/ipc.h>
#include <cstring>
#include <sys/msg.h>
#include "messages.h"
#include "logging.h"
#include "Car.h"
#include <unistd.h>

const key_t COMMAND_QUEUE_KEY = getuid() + 1;
const key_t CAR_QUEUE_KEY = getuid() + 2;

int connectToQueue(long key) {
    int queueId = msgget(key, 0666 | IPC_CREAT);
    if (queueId == -1) {
        perror("Failed to connect to a queue");
        exit(1);
    }
    return queueId;
}

void addToCarQueue(int queueId, CarMessage *message) {
    if (msgsnd(queueId, message, sizeof(CarMessage) - sizeof(long), 0) == -1) {
        perror("Failed to add a car to the queue");
        exit(1);
    }
}

void addToCommandQueue(int queueId, CommandMessage *message) {
    if (msgsnd(queueId, message, sizeof(CommandMessage) - sizeof(long), 0) == -1) {
        perror("Failed to add a command to the queue");
        exit(1);
    }
}

int receiveFromCarQueue(int queueId, CarMessage *message, int direction) {
    return msgrcv(queueId, message, sizeof(CarMessage) - sizeof(long), direction, 0666 | IPC_NOWAIT);
}

void receiveFromCommandQueue(int queueId, CommandMessage *message, int registration) {
    if (msgrcv(queueId, message, sizeof(CommandMessage) - sizeof(long), registration, 0) == -1) {
        perror("Failed to receive from the command queue");
        exit(1);
    }
}

void clearQueue(int queueId) {
    if (msgctl(queueId, IPC_RMID, nullptr) == -1) {
        perror("Failed to clear queue");
        exit(1);
    }
}

#endif
