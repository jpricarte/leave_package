//
// Created by jpricarte on 12/07/22.
//

#include "user.h"
#include "lp_exceptions.h"

namespace user {


    const std::string &User::getUsername() const {
        return username;
    }

    void User::setUsername(const std::string &username) {
        User::username = username;
    }

    sockaddr_in *User::getFstDevice() const {
        return fst_device;
    }

    void User::setFstDevice(sockaddr_in *fstDevice) {
        fst_device = fstDevice;
    }

    sockaddr_in *User::getSndDevice() const {
        return snd_device;
    }

    void User::setSndDevice(sockaddr_in *sndDevice) {
        snd_device = sndDevice;
    }

    int User::tryConnect() {
        return 0;
    }


    /** USER MANAGER METHODS **/

    User *UserManager::createUser(const std::string& username) {
        auto got_in = user::registered_user_list_semaphore.try_acquire_for(std::chrono::seconds (MAX_WAIT));
        if (got_in)
        {
            auto user_it = this->registered_users.find(username);

            if (user_it == this->registered_users.end())
            {
                User *user = new User(username);
                this->registered_users[username] = *user;
                return user;
            }
            User *user = &(this->registered_users[username]);
            user::registered_user_list_semaphore.release();
            return user;
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

    bool UserManager::userExists(const std::string username) {
        auto got_in = user::registered_user_list_semaphore.try_acquire_for(std::chrono::seconds (MAX_WAIT));

        if (got_in) {
            auto user_it = this->registered_users.find(username);
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