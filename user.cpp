//
// Created by jpricarte on 12/07/22.
//

#include "user.h"

namespace user {


    const std::string &User::getUsername() const {
        return username;
    }

    void User::setUsername(const std::string &username) {
        User::username = username;
    }

    void User::tryConnect(int sock_fd, communication::Transmitter* transmitter) {
        bool avaliable = avaliable_devices_semaphore->try_acquire();
        if (avaliable)
        {
            devices_sockets_semaphore->acquire();
            devices_sockets[sock_fd] = transmitter;
            devices_sockets_semaphore->release();
        }
        else
        {
            throw TooManyConnections();
        }
    }

    void User::disconnect(int sock_fd) {
        devices_sockets_semaphore->acquire();
        devices_sockets.erase(sock_fd);
        devices_sockets_semaphore->release();

        avaliable_devices_semaphore->release();
    }

    FileManager *User::getFileManager() const {
        return file_manager;
    }

    int User::countDevices() {
        devices_sockets_semaphore->acquire();
        int num_users = devices_sockets.size();
        devices_sockets_semaphore->release();
        return num_users;
    }

    bool User::isUniqueDevice() {
        devices_sockets_semaphore->acquire();
        int num_users = devices_sockets.size();
        bool isUnique = num_users == 1;
        devices_sockets_semaphore->release();
        return isUnique;
    }


    /** USER MANAGER METHODS **/

    UserManager::UserManager() {
        registered_users_semaphore = new std::binary_semaphore(1);
        registered_users = std::map<std::string, User*>();
    }

    User* UserManager::findOrCreateUser(const std::string &username) {
        registered_users_semaphore->acquire();
        auto user_it = registered_users.find(username);
        if (user_it != registered_users.end())
        {
            registered_users_semaphore->release();
            return user_it->second;
        }
        else
        {
            registered_users[username] = new User(username);
            registered_users_semaphore->release();
            return registered_users[username];
        }
    }
} // user