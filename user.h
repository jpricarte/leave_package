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

#include "lp_exceptions.h"

namespace user {
    const int MAX_WAIT = 5; // time in seconds
    static std::binary_semaphore active_user_list_semaphore(1);
    static std::binary_semaphore registered_user_list_semaphore(1);

    class User {
        std::string username;
        struct sockaddr_in fst_device, snd_device;

//  Default methods and overloads
    public:
        inline explicit User(std::string username) : username(std::move(username)) {};

        User();

        virtual ~User();

        const std::string &getUsername() const;

        void setUsername(const std::string &username);

        const sockaddr_in &getFstDevice() const;

        void setFstDevice(const sockaddr_in &fstDevice);

        const sockaddr_in &getSndDevice() const;

        void setSndDevice(const sockaddr_in &sndDevice);

    };


    /*
     * Essa classe funciona como o problema dos leitores/escritores
     * Ao criar/deletar usuário, deve-se ter acesso exclusivo
     * Ao Consultar, podemos ter multiplos acessos
     * TODO: arrumar isso para funcionar como deveria
     */
    class UserManager {
        std::map<std::string, User> registered_users;

    public:
        void deleteUser(const User& user);
        bool userExists(const User& user);
        void createUser(const User& user);

    public:
        UserManager();
    };

} // user

#endif //LEAVE_PACKAGE_USER_H
