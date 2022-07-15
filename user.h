//
// Created by jpricarte on 12/07/22.
//

#ifndef LEAVE_PACKAGE_USER_H
#define LEAVE_PACKAGE_USER_H

/*
 * User-related defines
*/
// Number of devices related to a single user

#include <string>
#include <vector>
#include <unordered_map>
#include <ostream>
#include <utility>
#include <semaphore>
#include "lp_exceptions.h"

namespace user {
    const int MAX_WAIT = 10; // time in seconds
    static std::binary_semaphore active_user_list_semaphore(1);

    class User {
        std::string username;
        std::string fst_device_ip;
        std::string snd_device_ip;

//  Default methods and overloads
    public:
        inline explicit User(std::string username) : username(std::move(username)) {};

        User();

        virtual ~User();

        const std::string &getUsername() const;

        void setUsername(const std::string &username);

        const std::string &getFstDeviceIp() const;

        void setFstDeviceIp(const std::string &fstDeviceIp);

        const std::string &getSndDeviceIp() const;

        void setSndDeviceIp(const std::string &sndDeviceIp);

        friend std::ostream &operator<<(std::ostream &os, const User &user1);

    };

    class UserManager {
        std::unordered_map<std::string, User> registered_users;
        std::unordered_map<std::string, User> active_users;

    public:
        void insertUser(const User& user);
        void removeUser(const User& user);
        bool isUserActive(const User& user);

    public:
        UserManager();
    };

} // user

#endif //LEAVE_PACKAGE_USER_H
