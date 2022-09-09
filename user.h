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
#include <queue>
#include <map>
#include <ostream>
#include <utility>
#include <semaphore>
#include <condition_variable>

#include <netinet/in.h>
#include <exception>
#include "fileManager.h"
#include "communication.h"
#include "UpdateHandler.h"

namespace user {

    class User {
        static const int MAX_USERS = 2;
        std::string username;
        // semaphore to guarantee a max num of Users
        std::counting_semaphore<MAX_USERS>* avaliable_devices_semaphore;
        // semaphore to avoid concurrent error in devices_command_sockets
        std::binary_semaphore* devices_sockets_semaphore;
        // maping (socket_fd, transmitter)
        std::map<int, communication::Transmitter*> devices_command_sockets;
        std::map<int, UpdateHandler*> devices_update_handler;

        FileManager* file_manager;

        // counting semaphore to sync
        std::counting_semaphore<INT16_MAX>* last_operations_semaphore;
        // A queue of command to send to devices
        std::vector<communication::CommandRecord> last_operations;
        // had_operation cv to wakeup sync_threads
        std::condition_variable* had_operation_cv;

//  Default methods and overloads
    public:

        inline explicit User(const std::string& username) : username(username) {
            avaliable_devices_semaphore = new std::counting_semaphore<2>(2);
            devices_sockets_semaphore = new std::binary_semaphore(1);
            file_manager = new FileManager(username);
            last_operations_semaphore = new std::counting_semaphore<INT16_MAX>(0);
            last_operations = std::vector<communication::CommandRecord>();
            had_operation_cv = new std::condition_variable();
        };

        inline virtual ~User() = default;

        // WARNING: O método é uma seção crítica
        void tryConnect(int command_socket, communication::Transmitter* command_transmitter,
                        UpdateHandler* update_handler); // try to get a socket or return -1 if user is full
        void disconnect(int command_socket); // try to get a socket or return -1 if user is full
        unsigned long countDevices();
        bool isUniqueDevice();
        UpdateHandler* getUpdateHandler(int curr_command_sock);
        UpdateHandler* getOtherDeviceUpdateHandler(int curr_command_sock);

        const std::string &getUsername() const;
        void setUsername(const std::string &username);

        FileManager *getFileManager() const;
    };


    /*
     * Essa classe serve apenas para salvar usuarios,
     * não é necessário apagar.
     */
    class UserManager {

        std::binary_semaphore* registered_users_semaphore;
        std::map<std::string, User*> registered_users;

        // equivalente a leitura

    public:
        // ambos são equivalentes à escrita
        User* findOrCreateUser(const std::string& username);

        UserManager();
    };

    struct TooManyConnections : public std::exception {
        [[nodiscard]] const char *what() const noexcept override {
            return "All conections avaliables are in use";
        }
    };

    struct SemaphoreOverused : public std::exception {
        [[nodiscard]] const char *what() const noexcept override {
            return "Waiting too long for semaphore";
        }
    };
} // user

#endif //LEAVE_PACKAGE_USER_H