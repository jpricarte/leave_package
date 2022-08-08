//
// Created by jpricarte on 01/08/22.
//

#include <fstream>
#include "commandHandler.h"
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

commandHandler::commandHandler(communication::Transmitter *transmitter, user::User *user) : transmitter(transmitter),
                                                                                            user(user) {
    in_use_semaphore = new std::counting_semaphore<1>(1);
}

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

    in_use_semaphore->acquire();  // Delete and Upload releases internally
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
        default:
            break;
    }
    in_use_semaphore->release();

}

void commandHandler::handleUploadFile(const std::string &filename, unsigned long total_size) {
    std::cout << "Remember, it's not syncing yet" << std::endl;

    std::string tmp_name = '.' + filename + '_' + user->getUsername() + ".tmp";
    std::ofstream tmp_file{tmp_name, std::ofstream::binary};

    if (tmp_file)
    {
        saveDataFlow(tmp_file);
        tmp_file.close();
        user->getFileManager()->moveFile(tmp_name, filename);
    }
}

void commandHandler::handleDeleteFile(const std::string &filename) {
    user->getFileManager()->deleteFile(filename);
}

void commandHandler::handleGetSyncDir() {
    auto files = split_str(user->getFileManager()->listFiles(), ',');
    for (const auto& filename : files)
    {
        size_t filesize;
        std::string fpath = user->getFileManager()->getPath();
        fpath += '/' + filename;
        try {
            filesize = std::filesystem::file_size(fpath);
        } catch (std::filesystem::filesystem_error& e) {
            std::cerr << e.what() << std::endl;
            continue;
        }
        // primeiro, envia o nome e outros metadados (se precisar) do arquivo
        communication::Packet metadata_packet {
                communication::DOWNLOAD,
                1,
                filesize,
                (unsigned int) strlen(filename.c_str()),
                (char*) filename.c_str()
        };

        try {
            transmitter->sendPackage(metadata_packet);
        } catch (communication::SocketWriteError& e) {
            std::cerr << e.what() << std::endl;
        }

        handleDownloadFile(filename);
    }
    
    try {
        transmitter->sendPackage(communication::SUCCESS);
    } catch (communication::SocketWriteError& e) {
        std::cerr << e.what() << std::endl;
    }
}

void commandHandler::handleListServer() {
    auto file_list = user->getFileManager()->listFiles();
    communication::Packet packet = {
            communication::GET_SYNC_DIR,
            1,
            file_list.size(),
            (unsigned int) strlen(file_list.c_str()),
            (char*) file_list.c_str()
    };

    try {
        transmitter->sendPackage(packet);
    } catch (communication::SocketWriteError& e) {
        std::cerr << e.what() << std::endl;
    }
}

void commandHandler::handleDownloadFile(const std::string &filename) {
    std::string fpath = user->getFileManager()->getPath();
    fpath += '/' + filename;
    auto filesize = std::filesystem::file_size(fpath);
    std::ifstream file(fpath, std::ios::binary);
    if (!file)
    {
        return;
    }

    // depois, envia o arquivo em partes de 255 Bytes
    const unsigned int BUF_SIZE = 255;
    char buf[BUF_SIZE] = {};
    unsigned int i = 2;
    while(!file.eof())
    {
        bzero(buf, BUF_SIZE);
        file.read(buf, BUF_SIZE);
        communication::Packet data_packet {
                communication::DOWNLOAD,
                i,
                filesize,
                (unsigned int) strlen(buf),
                buf
        };

        try {
            transmitter->sendPackage(data_packet);
        } catch (communication::SocketWriteError& e) {
            std::cerr << e.what() << std::endl;
            break;
        }

        try {
            communication::Packet packet = transmitter->receivePackage();
            if (packet.command == communication::OK) {
                i++;
            } else {
                break;
            }
        } catch (communication::SocketReadError& e) {
            std::cerr << e.what() << std::endl;
            break;
        }
    }

    try {
        transmitter->sendPackage(communication::SUCCESS);
    } catch (communication::SocketWriteError& e) {
        std::cerr << e.what() << std::endl;
    }

}

void commandHandler::saveDataFlow(std::ofstream &tmp_file) {
    auto last_command = communication::UPLOAD;
    while(last_command != communication::OK)
    {
        try {
            auto packet = transmitter->receivePackage();
            last_command = packet.command;
            if (last_command == communication::UPLOAD)
            {
                tmp_file.write((char*)packet._payload, packet.length);
            }
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
}


void commandHandler::syncUploadInDevices() {

}

