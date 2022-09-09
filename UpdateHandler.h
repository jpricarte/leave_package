//
// Created by jpricarte on 07/09/22.
//

#ifndef LEAVE_PACKAGE_UPDATEHANDLER_H
#define LEAVE_PACKAGE_UPDATEHANDLER_H


#include "communication.h"
#include "fileManager.h"

class UpdateHandler {
    std::binary_semaphore* in_use_semaphore;
    communication::Transmitter* transmitter;
    FileManager* file_manager;

public:
    UpdateHandler(communication::Transmitter *transmitter, FileManager *fileManager);

    void sendFile(const std::string& filename);
    void deleteFile(const std::string& filename);

    communication::Transmitter *getTransmitter() const;
};


#endif //LEAVE_PACKAGE_UPDATEHANDLER_H
