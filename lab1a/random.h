#ifndef LAB1A_RANDOM_H
#define LAB1A_RANDOM_H

#include <random>
#include <random>

int randomInt(int from, int toInclusive) {
    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(from, toInclusive);
    return dist(rng);
}

#endif
