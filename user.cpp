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

    void User::tryConnect(int command_socket, communication::Transmitter* command_transmitter,
                          UpdateHandler* update_handler) {
        bool avaliable = avaliable_devices_semaphore->try_acquire();
        if (avaliable)
        {
            devices_sockets_semaphore->acquire();
            devices_command_sockets[command_socket] = command_transmitter;
            // Use the same socket to associate them
            devices_update_handler[command_socket] = update_handler;
            devices_sockets_semaphore->release();
        }
        else
        {
            throw TooManyConnections();
        }
    }

    void User::disconnect(int command_socket) {
        devices_sockets_semaphore->acquire();
        devices_command_sockets.erase(command_socket);
        devices_update_handler.erase(command_socket);
        devices_sockets_semaphore->release();

        avaliable_devices_semaphore->release();
    }

    FileManager *User::getFileManager() const {
        return file_manager;
    }

    unsigned long User::countDevices() {
        devices_sockets_semaphore->acquire();
        auto num_users = devices_command_sockets.size();
        devices_sockets_semaphore->release();
        return num_users;
    }

    bool User::isUniqueDevice() {
        devices_sockets_semaphore->acquire();
        int num_users = devices_command_sockets.size();
        bool isUnique = num_users == 1;
        devices_sockets_semaphore->release();
        return isUnique;
    }

    UpdateHandler *User::getOtherDeviceUpdateHandler(int curr_command_sock) {
        devices_sockets_semaphore->acquire();
        for (auto sock_trans : devices_update_handler)
        {
            if (std::get<0>(sock_trans) == curr_command_sock) continue;
            devices_sockets_semaphore->release();
            return std::get<1>(sock_trans);
        }
        devices_sockets_semaphore->release();
        return nullptr;
    }

    UpdateHandler *User::getUpdateHandler(int curr_command_socket) {
        return devices_update_handler[curr_command_socket];
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
