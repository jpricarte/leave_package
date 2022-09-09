//
// Created by jpricarte on 07/09/22.
//

#include <fstream>
#include "UpdateHandler.h"


UpdateHandler::UpdateHandler(communication::Transmitter *transmitter, FileManager *fileManager) : transmitter(
        transmitter), file_manager(fileManager) {
    in_use_semaphore = new std::binary_semaphore(1);
}

void UpdateHandler::sendFile(const std::string &filename) {
    std::string fpath = file_manager->getPath();
    fpath += '/' + filename;
    auto filesize = std::filesystem::file_size(fpath);

    in_use_semaphore->acquire();

    communication::Packet metadata_packet {
            communication::SYNC_UPLOAD,
            1,
            filesize,
            (unsigned int) strlen(filename.c_str()),
            (char*) filename.c_str()
    };
    try {
        transmitter->sendPacket(metadata_packet);
    } catch (communication::SocketWriteError& e) {
        std::cerr << e.what() << std::endl;
    }

    std::ifstream file(fpath, std::ios::binary);
    if (!file)
    {
        return;
    }

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

    in_use_semaphore->release();
}

communication::Transmitter *UpdateHandler::getTransmitter() const {
    return transmitter;
}

void UpdateHandler::deleteFile(const std::string &filename) {
    in_use_semaphore->acquire();
    communication::Packet delete_packet {
            communication::SYNC_DELETE,
            1,
            strlen(filename.c_str()),
            (unsigned int) strlen(filename.c_str()),
            (char*) filename.c_str()
    };
    try {
        transmitter->sendPacket(delete_packet);
    } catch(communication::SocketWriteError& e) {
        std::cout << e.what() << std::endl;
    }
    in_use_semaphore->release();
}
