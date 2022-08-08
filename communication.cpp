//
// Created by jpricarte on 28/07/22.
//


#include "communication.h"

namespace communication {

    void showPacket(const Packet& packet)
    {
        std::cout << "{" << std::endl;
        std::cout << "seqn: " << packet.seqn << std::endl;
        std::cout << "total size: " << packet.total_size << std::endl;
        std::cout << "length: " << packet.length << std::endl;
        std::cout << "payload: " << std::endl;
        std::cout << packet._payload << std::endl;
        std::cout << "}" << std::endl;

    }

    Transmitter::Transmitter(sockaddr_in *clientAddr, int socketfd) : client_addr(clientAddr), socketfd(socketfd) {
        socket_semaphore = new std::counting_semaphore<1>(1);
    }

    Transmitter::~Transmitter() {
        close(this->socketfd);
    }

    void Transmitter::sendPackage(const Packet &packet) {
        auto* sendable_packet = new Packet{
                packet.command,
                htons(packet.seqn),
                htonl(packet.total_size),
                htons(packet.length),
                nullptr
        };

        // send command
        socket_semaphore->acquire();
        auto res = write(socketfd, sendable_packet, sizeof(Packet));
        if (res < 0)
        {
            socket_semaphore->release();
            throw SocketWriteError();
        }
        res = write(socketfd,  packet._payload, packet.length);
        if (res < 0)
        {
            socket_semaphore->release();
            throw SocketWriteError();
        }
        socket_semaphore->release();
    }

    Packet Transmitter::receivePackage() {
        Packet packet{};
        socket_semaphore->acquire();
        auto res = read(socketfd, &packet, sizeof(Packet));
        if (res < 0) {
            socket_semaphore->release();
            std::cerr << "Error in header" << std::endl;
            throw SocketReadError();
        }
        packet.seqn = ntohs(packet.seqn);
        packet.total_size = ntohl(packet.total_size);
        packet.length = ntohs(packet.length);
        packet._payload = new char[packet.length+1];
        bzero(packet._payload, packet.length+1);
        res = read(socketfd, packet._payload, packet.length);
        if (res < 0) {
            socket_semaphore->release();
            std::cerr << "Error in payload" << std::endl;
            throw SocketReadError();
        }
        socket_semaphore->release();

        return packet;
    }

    sockaddr_in *Transmitter::getClientAddr() const {
        return client_addr;
    }

    int Transmitter::getSocketfd() const {
        return socketfd;
    }
} // communication