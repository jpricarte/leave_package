//
// Created by jpricarte on 01/08/22.
//

#include <fstream>
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

void commandHandler::handlePackage(communication::Packet& packet) {
    auto command = packet.command;

    switch (command) {
        case communication::UPLOAD:
            handleUploadFile(packet._payload);
            break;
        case communication::DELETE:
            handleDeleteFile(packet._payload);
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

    std::string tmp_name = '.' + filename + '_' + user->getUsername() + ".tmp";
    std::ofstream tmp_file{tmp_name, std::ofstream::binary};

    if (tmp_file)
    {
        auto last_command = communication::UPLOAD;
        while(last_command != communication::OK)
        {
            try {
                auto* packet = new communication::Packet;
                *packet = transmitter->receivePackage();
                last_command = packet->command;
                if (last_command == communication::UPLOAD)
                {
                    tmp_file.write(packet->_payload, packet->length);
                }
                delete packet;
            } catch (communication::SocketReadError& e) {
                std::cerr << e.what() << std::endl;
                break;
            } catch (...) {
                try {
                    transmitter->sendPackage(communication::ERROR);
                } catch (communication::SocketWriteError& e2) {
                    std::cerr << e2.what() << std::endl;
                    break;
                }
            }

            if (last_command != communication::OK)
            {
                try {
                    transmitter->sendPackage(communication::SUCCESS);
                } catch (communication::SocketWriteError& e) {
                    std::cerr << e.what() << std::endl;
                    break;
                }
            }
        }
        tmp_file.close();
        user->getFileManager()->moveFile(tmp_name, filename);
    }

}
void commandHandler::handleDeleteFile(const std::string &filename) {
    user->getFileManager()->deleteFile(filename);
}

void commandHandler::handleGetSyncDir() {
    std::cout << "handleGetSyncDir(...) Not implemented yet" << std::endl;
}

void commandHandler::handleListServer() {
    auto file_list = user->getFileManager()->listFiles();
    communication::Packet packet = {
            communication::LIST_SERVER,
            1,
            file_list.size(),
            (unsigned int) file_list.size(),
            (char*) file_list.c_str()
    };

    try {
        transmitter->sendPackage(packet);
    } catch (communication::SocketWriteError& e) {
        std::cerr << e.what() << std::endl;
    }

}

void commandHandler::handleExit() {

}


