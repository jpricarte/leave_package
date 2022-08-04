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
#include <map>
#include <ostream>
#include <utility>
#include <semaphore>

#include <netinet/in.h>
#include <exception>
#include "fileManager.h"

namespace user {

    class User {
        static const int MAX_USERS = 2;
        std::string username;
        // semaphore to guarantee a max num of Users
        std::counting_semaphore<MAX_USERS>* avaliable_devices_semaphore;
        // semaphore to avoid concurrent error in devices_sockets
        std::binary_semaphore* devices_sockets_semaphore;
        // maping (socket_fd, socket_addr)
        std::map<int, sockaddr_in*> devices_sockets;

        //TODO: remover comentário abaixo quando estiver resolvido
        //WARNING: O fileManger não está thread_safe
        FileManager* file_manager;

        // fileWatcher() fica observando os arquivos na pasta do usuário

//  Default methods and overloads
    public:

        inline explicit User(const std::string& username) : username(username) {
            avaliable_devices_semaphore = new std::counting_semaphore<2>(2);
            devices_sockets_semaphore = new std::binary_semaphore(1);
            file_manager = new FileManager(username);

            // Inicia uma thread para olhar arquivos
        };

        inline virtual ~User() = default;

        // WARNING: O método é uma seção crítica
        void tryConnect(int sock_fd, sockaddr_in* addr_in); // try to get a socket or return -1 if user is full
        void disconnect(int sock_fd); // try to get a socket or return -1 if user is full

        const std::string &getUsername() const;
        void setUsername(const std::string &username);

        FileManager *getFileManager() const;

        // sendToDevices(const& filename) envia o arquivo para TODOS os sockets associados

        // deleteInDevices(const& filename) envia um comando para deletar o arquivo em TODOS os sockets associados
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
