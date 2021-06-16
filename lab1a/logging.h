#ifndef LAB1A_LOGGING_H
#define LAB1A_LOGGING_H

#include <string>
#include <iostream>

void log(const std::string &text) {
    std::cout << text << std::endl;
}

void error(const std::string &text) {
    std::cerr << text << std::endl;
}

#endif
