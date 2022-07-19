//
// Created by jpricarte on 12/07/22.
//

#include "user.h"
#include "lp_exceptions.h"

namespace user {

    User::User() = default;

    User::~User() = default;

    const std::string &User::getUsername() const {
        return username;
    }

    void User::setUsername(const std::string &username) {
        User::username = username;
    }

    const std::string &User::getFstDeviceIp() const {
        return fst_device_ip;
    }

    void User::setFstDeviceIp(const std::string &fstDeviceIp) {
        fst_device_ip = fstDeviceIp;
    }

    const std::string &User::getSndDeviceIp() const {
        return snd_device_ip;
    }

    void User::setSndDeviceIp(const std::string &sndDeviceIp) {
        snd_device_ip = sndDeviceIp;
    }

    std::ostream &operator<<(std::ostream &os, const User &user1) {
        os << "username: " << user1.username << " fst_device_ip: " << user1.fst_device_ip << " snd_device_ip: "
           << user1.snd_device_ip;
        return os;
    }

    /** USER MANAGER METHODS **/

    void UserManager::createUser(const User &user) {
        auto got_in = user::registered_user_list_semaphore.try_acquire_for(std::chrono::seconds (MAX_WAIT));
        if (got_in)
        {
            auto user_it = this->registered_users.find(user.getUsername());

            if (user_it == this->registered_users.end())
            {
                this->registered_users[user.getUsername()] = user;
            }
            user::registered_user_list_semaphore.release();
        }
        else
        {
            throw SemaphoreOverused();
        }
    }

    void UserManager::deleteUser(const User &user) {
        auto got_in = user::registered_user_list_semaphore.try_acquire_for(std::chrono::seconds (MAX_WAIT));
        if (got_in)
        {
            auto user_it = this->registered_users.find(user.getUsername());

            if (user_it == this->registered_users.end())
            {
                this->registered_users.erase(user.getUsername());
            }
            user::registered_user_list_semaphore.release();
        }
        else
        {
            throw SemaphoreOverused();
        }
    }

    bool UserManager::userExists(const User &user) {
        auto got_in = user::registered_user_list_semaphore.try_acquire_for(std::chrono::seconds (MAX_WAIT));

        if (got_in) {
            auto user_it = this->registered_users.find(user.getUsername());
            user::registered_user_list_semaphore.release();

            if (user_it == this->registered_users.end())
                return false;
            else
                return false;
        }
        else {
            throw SemaphoreOverused();
        }
    }

    UserManager::UserManager() {
        registered_users = std::map<std::string, User>();
    }
} // user