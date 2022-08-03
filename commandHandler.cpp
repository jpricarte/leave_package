//
// Created by jpricarte on 01/08/22.
//

#include "commandHandler.h"
#include "communication.h"


commandHandler::commandHandler(communication::Transmitter *transmitter, user::User *user) : transmitter(transmitter),
                                                                                            user(user) {}

void commandHandler::handleIncome() {
    communication::Command last_command = communication::NOP;
    while(last_command != communication::EXIT)
    {
        try {
            auto packet = transmitter->receivePackage();
            last_command = packet.command;
            handlePackage(packet);
        } catch (communication::SocketReadError& e) {
            std::cerr << e.what() << std::endl;
        }
    }
}

void commandHandler::handlePackage(communication::Packet packet) {
    auto command = packet.command;
    auto content = std::string(packet._payload);

    switch (command) {
        case communication::UPLOAD:
            handleUploadFile(content);
            break;
        case communication::DOWNLOAD:
            handleDownloadFile(content);
            break;
        case communication::DELETE:
            handleDeleteFile(content);
            break;
        case communication::GET_SYNC_DIR:
            handleGetSyncDir();
            break;
        case communication::LIST_SERVER:
            handleListServer();
            break;
        default:
            break;
    }
}

void commandHandler::handleUploadFile(const std::string &filename) {
    std::cout << "Remember, it's not syncing yet" << std::endl;

    try {
        auto file_packet = transmitter->receivePackage();
        auto file_content = std::string(file_packet._payload);
        user->getFileManager()->createFile(filename, file_content);
        // Save file with filename and content
    } catch (communication::SocketReadError& e) {
        std::cerr << e.what() << std::endl;
    }
}

void commandHandler::handleDownloadFile(const std::string &filename) {
    std::cout << "handleDownloadFile(...) Not implemented yet" << std::endl;
}

void commandHandler::handleDeleteFile(const std::string &filename) {
    std::cout << "handleDeleteFile(...) Not implemented yet" << std::endl;
}

void commandHandler::handleGetSyncDir() {
    std::cout << "handleGetSyncDir(...) Not implemented yet" << std::endl;
}

void commandHandler::handleListServer() {
    std::cout << "handleListServer(...) Not implemented yet" << std::endl;
}


