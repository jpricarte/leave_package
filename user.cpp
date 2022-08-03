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

    void User::tryConnect(int sock_fd, sockaddr_in *addr_in) {
        bool avaliable = avaliable_devices_semaphore->try_acquire();
        if (avaliable)
        {
            devices_sockets_semaphore->acquire();
            devices_sockets[sock_fd] = addr_in;
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