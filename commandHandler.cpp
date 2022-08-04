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

void commandHandler::handlePackage(communication::Packet& packet) {
    auto command = packet.command;
    auto content = std::string(packet._payload);
    delete packet._payload;
    std::stringstream data_stream{content};

    switch (command) {
        case communication::UPLOAD:
            handleUploadFile(data_stream);
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

void commandHandler::handleUploadFile(std::stringstream& datastream) {
    std::cout << "Remember, it's not syncing yet" << std::endl;
    std::string filename{};
    std::string file_content{};
    getline(datastream, filename);
    while(!datastream.eof())
    {
        std::string buf;
        getline(datastream, buf);
        file_content += buf + "\n";
    }
    try {
        user->getFileManager()->createFile(filename, file_content);
        // Save file with filename and file_content
    } catch (communication::SocketReadError& e) {
        std::cerr << e.what() << std::endl;
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


