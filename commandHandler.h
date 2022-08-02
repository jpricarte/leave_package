//
// Created by jpricarte on 01/08/22.
//

#ifndef LEAVE_PACKAGE_COMMANDHANDLER_H
#define LEAVE_PACKAGE_COMMANDHANDLER_H


#include "communication.h"

class commandHandler {
    communication::Transmitter* transmitter;

    void handlePackage(communication::Packet packet);
    void handleUploadFile(const std::string& filename);
    void handleDownloadFile(const std::string& filename);
    void handleDeleteFile(const std::string& filename);
    void handleGetSyncDir();
    void handleListServer();


public:
    explicit commandHandler(communication::Transmitter *transmitter);
    void handleIncome();
};


#endif //LEAVE_PACKAGE_COMMANDHANDLER_H
