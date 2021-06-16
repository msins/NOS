#ifndef LAB1A_MESSAGES_H
#define LAB1A_MESSAGES_H

#include <string>
#include "Car.h"

const char PASSING_STATE[] = "Passing";
const char PASSED_STATE[] = "Passed";
const long DIRECTION_0 = 1;
const long DIRECTION_1 = 2;

struct CommandMessage {
    long type;
    Car car;
    char state[20];

    std::string toString() {
        return car.toString() + state;
    }
};

struct CarMessage {
    long type;
    Car car;

    std::string toString() {
        return car.toString();
    }
};

#endif
