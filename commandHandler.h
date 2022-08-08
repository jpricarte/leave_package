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

    std::binary_semaphore *in_use_semaphore;

    std::mutex *need_update_mutex;
    std::condition_variable need_update_cv;
    std::condition_variable was_updated_cv;
    bool was_updated;
    std::string updated_filename;
    int sender_fd;

    bool still_working;

    void handlePackage(communication::Packet& packet);
    void handleUploadFile(const std::string &filename, unsigned long total_size);
    void handleDeleteFile(const std::string& filename);
    void handleGetSyncDir();
    void handleListServer();
    void handleDownloadFile(const std::string &filename);

    void saveDataFlow(std::ofstream& tmp_file);


public:
    commandHandler(communication::Transmitter *transmitter, user::User *user);
    void syncUploadInDevices();

    void handleIncome();

};


#endif //LEAVE_PACKAGE_COMMANDHANDLER_H
