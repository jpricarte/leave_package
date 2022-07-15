//
// Created by jpricarte on 15/07/22.
//

#ifndef LEAVE_PACKAGE_LP_EXCEPTIONS_H
#define LEAVE_PACKAGE_LP_EXCEPTIONS_H

#include <exception>

struct SemaphoreOverused : public std::exception {
    const char *what() const noexcept override {
        return "Waiting too long for semaphore";
    }
};

#endif //LEAVE_PACKAGE_LP_EXCEPTIONS_H
