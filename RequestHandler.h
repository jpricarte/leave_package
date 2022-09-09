//
// Created by jpricarte on 01/08/22.
//

#ifndef LEAVE_PACKAGE_REQUESTHANDLER_H
#define LEAVE_PACKAGE_REQUESTHANDLER_H


#include "communication.h"
#include "user.h"

class RequestHandler {
    communication::Transmitter* transmitter;
    user::User* user;
    std::binary_semaphore* in_use_semaphore;

    void handlePackage(communication::Packet& packet);
    void handleUploadFile(const std::string &filename, std::size_t file_size);
    void handleDeleteFile(const std::string& filename);
    void handleGetSyncDir();
    void handleListServer();
    void handleDownloadFile(const std::string &filename);

    void saveDataFlow(std::ofstream& tmp_file, std::size_t file_size);


public:
    RequestHandler(communication::Transmitter *transmitter, user::User *user);
    static void sendFileThroughTransmitter(communication::Transmitter *transmitter, const std::string& fpath);

    void handleIncome();
    void syncWithOtherDevice();

};


#endif //LEAVE_PACKAGE_REQUESTHANDLER_H