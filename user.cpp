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

        last_operations.insert(last_operations.begin(), {sock_fd, communication::EXIT, ""});
        last_operations_semaphore->release();

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


// Not Used
    void User::pushOperationToSync(communication::CommandRecord &command_record) {
        last_operations.push_back(command_record);
        last_operations_semaphore->release();
    }

    /*
     * Lógica por trás:
     * Ou o dispositivo enviou o record, ou precisa atualizar localmente
     * Caso seja quem enviou, retorna um commando record com NOP e sock 0
     * Caso contrário, retorna o record normal e apaga esse elemento.
     * Está tudo bem que quem enviou o record não veja a mensagem, pois ela
     * seria ignorada de qualquer forma.
     * */
    communication::CommandRecord User::popOperationToSync(int req_sock_fd) {
        communication::CommandRecord cr {0, communication::NOP, ""};

        last_operations_semaphore->acquire();
        cr = last_operations.front();

        if (cr.command != communication::EXIT) { // Normal operation
            if (req_sock_fd == cr.sock_fd) {
                cr.sock_fd = 0;
                cr.command = communication::NOP;
                if (countDevices() > 1) {
                    last_operations_semaphore->release();
                }
            } else {
                last_operations.erase(last_operations.begin());
            }
        } else {
            if (req_sock_fd == cr.sock_fd) { // Exit use the inverse logic
                last_operations.erase(last_operations.begin());
            } else {
                cr.sock_fd = 0;
                cr.command = communication::NOP;
                if (countDevices() > 1) {
                    last_operations_semaphore->release();
                }
            }
        }

        return cr;
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
