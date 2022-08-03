//
// Created by jpricarte on 01/08/22.
//

#ifndef LEAVE_PACKAGE_COMMANDHANDLER_H
#define LEAVE_PACKAGE_COMMANDHANDLER_H


#include "communication.h"
#include "user.h"

class commandHandler {
    communication::Transmitter* transmitter;
    user::User* user;

    void handlePackage(communication::Packet packet);
    void handleUploadFile(const std::string& filename);
    void handleDownloadFile(const std::string& filename);
    void handleDeleteFile(const std::string& filename);
    void handleGetSyncDir();
    void handleListServer();


public:
    commandHandler(communication::Transmitter *transmitter, user::User *user);

    void handleIncome();
};


#endif //LEAVE_PACKAGE_COMMANDHANDLER_H