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
            user->pushOperationToSync(command_record);
            break;
        case communication::DELETE:
            handleDeleteFile(packet._payload);
            user->pushOperationToSync(command_record);
            break;
        case communication::GET_SYNC_DIR:
            handleGetSyncDir();
            break;
        case communication::LIST_SERVER:
            handleListServer();
            break;
        case communication::EXIT:
            user->pushOperationToSync(command_record);
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
    }
    else {
        std::cerr << filename << ": can't open" << std::endl;
    }
}

void RequestHandler::handleDeleteFile(const std::string &filename) {
    user->getFileManager()->deleteFile(filename);
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
    std::size_t remeaning_size = filesize;
    // depois, envia o arquivo em partes de 1023 Bytes
    const unsigned int BUF_SIZE = 1023;
    char buf[BUF_SIZE] = {};
    unsigned int i = 2;
    while(remeaning_size > 0)
    {
        bzero(buf, BUF_SIZE);
        auto read_size = std::min((unsigned long)BUF_SIZE, remeaning_size);
        remeaning_size -= read_size;
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
    std::size_t current_size = 0;
    while(last_command != communication::OK)
    {
        std::cout << current_size << " of " << file_size << std::endl;
        try {
            packet = transmitter->receivePacket();
            last_command = packet.command;
            current_size += packet.length;

            if (last_command == communication::UPLOAD)
            {
                tmp_file.write(packet._payload, packet.length);
//                current_size += packet.length;
            }
        } catch (communication::SocketReadError& e) {
            std::cerr << e.what() << std::endl;
            break;
        }
    }
    std::cout << "I finish" << std::endl;
}

void RequestHandler::syncWithOtherDevice() {
    communication::CommandRecord cr{0,communication::NOP,""};

    while (cr.command != communication::EXIT) {
        cr = user->popOperationToSync(transmitter->getSocketfd());
        if (cr.command == communication::NOP) continue;
        communication::Packet packet = {communication::NOP, 1,
                                        cr.filename.size(),
                                        (unsigned int) cr.filename.size(),
                                        (char*) cr.filename.c_str()
        };
        in_use_semaphore->acquire();
        switch(cr.command) {
            case communication::UPLOAD:
                packet.command = communication::SYNC_UPLOAD;
                try {
                    transmitter->sendPacket(packet);
                } catch (communication::SocketWriteError& e) {
                    std::cerr << e.what() << std::endl;
                }
                break;
            case communication::DELETE:
                packet.command = communication::SYNC_DELETE;
                try {
                    transmitter->sendPacket(packet);
                } catch (communication::SocketWriteError& e) {
                    std::cerr << e.what() << std::endl;
                }
                break;
            default:
                break;
        }
        in_use_semaphore->release();
    }
}
