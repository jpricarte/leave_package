//
// Created by jpricarte on 01/08/22.
//

#include <fstream>
#include "RequestHandler.h"
#include "communication.h"


// Code from: https://www.javatpoint.com/how-to-split-strings-in-cpp
std::vector<std::string> split_str( std::string const &str, const char delim)
{
    // create a stream from the string
    std::stringstream stream(str);

    std::string buf;
    std::vector <std::string> out{};
    while (std::getline(stream, buf, delim))
    {
        out.push_back(buf); // store the string in s2
    }
    return out;
}

RequestHandler::RequestHandler(communication::Transmitter *transmitter, user::User *user) : transmitter(transmitter),
                                                                                            user(user) {
    in_use_semaphore = new std::binary_semaphore(1);
}

void RequestHandler::handleIncome() {
    communication::Command last_command = communication::NOP;
    while(last_command != communication::EXIT)
    {
        try {
            auto packet = transmitter->receivePacket();
            last_command = packet.command;
            handlePackage(packet);
        } catch (communication::SocketReadError& e) {
            std::cerr << e.what() << std::endl;
        }
    }
}

void RequestHandler::handlePackage(communication::Packet& packet) {
    auto command = packet.command;

    communication::CommandRecord command_record{transmitter->getSocketfd(), command, packet._payload};

    in_use_semaphore->acquire();
    switch (command) {
        case communication::UPLOAD:
            handleUploadFile(packet._payload, packet.total_size);
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
        case communication::EXIT:
            break;
        default:
            break;
    }
    in_use_semaphore->release();

}

void RequestHandler::handleUploadFile(const std::string &filename, std::size_t file_size) {
    std::string tmp_name = '.' + filename + '_' + user->getUsername() + ".tmp";
    std::ofstream tmp_file{tmp_name, std::ofstream::binary};

    if (tmp_file)
    {
        saveDataFlow(tmp_file, file_size);
        tmp_file.close();
        user->getFileManager()->moveFile(tmp_name, filename);
        std::string final_path = user->getUsername() + "/" + filename;
        user->getUpdateHandler(transmitter->getSocketfd())->sendFile(filename);
        UpdateHandler* other_device_update_handler
            = user->getOtherDeviceUpdateHandler(transmitter->getSocketfd());
        if (other_device_update_handler != nullptr)
            std::thread(&UpdateHandler::sendFile, other_device_update_handler, filename).detach();
    }
    else {
        std::cerr << filename << ": can't open" << std::endl;
    }
}

void RequestHandler::handleDeleteFile(const std::string &filename) {
    user->getFileManager()->deleteFile(filename);
    user->getUpdateHandler(transmitter->getSocketfd())->deleteFile(filename);
    auto other_user_update_handler =
            user->getOtherDeviceUpdateHandler(transmitter->getSocketfd());
    if (other_user_update_handler != nullptr)
        other_user_update_handler->deleteFile(filename);
}

void RequestHandler::handleGetSyncDir() {
    auto files = user->getFileManager()->listFilesAndLastModified();
    for (const auto& file : files)
    {
        std::string fpath = user->getFileManager()->getPath();
        fpath += '/' + file.filename;

        size_t filesize = file_size(std::filesystem::path(fpath));
        // primeiro, envia o nome e outros metadados (se precisar) do arquivo
        std::string sendable = file.filename;
        communication::Packet metadata_packet {
                communication::DOWNLOAD,
                1,
                filesize,
                (unsigned int) strlen(sendable.c_str()),
                (char*) sendable.c_str()
        };
        try {
            transmitter->sendPacket(metadata_packet);
        } catch (communication::SocketWriteError& e) {
            std::cerr << e.what() << std::endl;
        }

        handleDownloadFile(file.filename);
    }
    
    try {
        transmitter->sendPacket(communication::SUCCESS);
    } catch (communication::SocketWriteError& e) {
        std::cerr << e.what() << std::endl;
    }
}

void RequestHandler::handleListServer() {
    auto file_list = user->getFileManager()->listFiles();
    communication::Packet packet = {
            communication::GET_SYNC_DIR,
            1,
            file_list.size(),
            (unsigned int) strlen(file_list.c_str()),
            (char*) file_list.c_str()
    };

    try {
        transmitter->sendPacket(packet);
    } catch (communication::SocketWriteError& e) {
        std::cerr << e.what() << std::endl;
    }
}

void RequestHandler::handleDownloadFile(const std::string &filename) {
    std::string fpath = user->getFileManager()->getPath();
    fpath += '/' + filename;

    auto filesize = std::filesystem::file_size(fpath);

    std::ifstream file(fpath, std::ios::binary);
    if (!file)
    {
        return;
    }
//    std::cout << filename << " - " << filesize << std::endl;
    std::size_t remaining_size = filesize;
    // depois, envia o arquivo em partes de 1024 Bytes
    const unsigned int BUF_SIZE = 1024;
    char buf[BUF_SIZE] = {};
    unsigned int i = 2;
    while(remaining_size > 0)
    {
        bzero(buf, BUF_SIZE);
        auto read_size = std::min((unsigned long)BUF_SIZE, remaining_size);
        remaining_size -= read_size;
        file.read(buf, (unsigned int) read_size);

        communication::Packet data_packet {
                communication::DOWNLOAD,
                i,
                filesize,
                (unsigned int) read_size,
                buf
        };

        try {
            transmitter->sendPacket(data_packet);
        } catch (communication::SocketWriteError& e) {
            std::cerr << e.what() << std::endl;
            break;
        }

        i++;
    }

    try {
        transmitter->sendPacket(communication::SUCCESS);
    } catch (communication::SocketWriteError& e) {
        std::cerr << e.what() << std::endl;
    }
}

void RequestHandler::saveDataFlow(std::ofstream &tmp_file, std::size_t file_size) {
    auto last_command = communication::UPLOAD;
    communication::Packet packet{};
    unsigned int last_seqn = 1;
    std::size_t current_size = 0;
    while(current_size < file_size)
    {
        try {
            packet = transmitter->receivePacket();
            last_command = packet.command;
            current_size += packet.length;

            if (last_command == communication::UPLOAD)
            {
                last_seqn++;
                tmp_file.write(packet._payload, packet.length);
            }
        } catch (communication::SocketReadError& e) {
            std::cerr << e.what() << std::endl;
            break;
        }
    }
}

void RequestHandler::sendFileThroughTransmitter(communication::Transmitter *transmitter, const std::string& fpath) {

}