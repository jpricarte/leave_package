//
// Created by santi on 02/08/2022.
//

#include <iostream>
#include <thread>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>

#include "client.h"
#include "communication.h"
#include "commandHandler.h"


using namespace std;

std::string Client::username;
int Client::server_socket;

pthread_t Client::input_handler_thread;
pthread_t Client::keep_alive_thread;

Monitor Client::monitor;

Client::Client(std::string username, std::string server_ip, std::string port)
{
    this->username = username;
    this->server_ip = server_ip;
    this->port = stoi(port);
    this->server_socket = -1;
};

Client::~Client()
{
    if (server_socket > 0)
        close(server_socket);
};

void Client::setupConnection()
{
    //TODO 1. HOW TO HANDLE LOGIN?
    //Try to create socket
    if ( (server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout <<"ERROR: unable to create socket";
        exit(0);
    }
    // Fill server socket address
    struct sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_address.sin_zero), 8);

    // Try to connect to server
    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        std::cout <<"ERROR: unable to connect to server";
        exit(0);
    }
    auto* transmitter = new communication::Transmitter(server_address, server_socket);
    //login packet
    int Packet_size = sizeof(communication::Packet);
    char * send_username = const_cast<char*>(username.c_str());
    communication::Packet* packet = (communication::Packet*)malloc(Packet_size);
    bzero((void*)packet, Packet_size);
    packet->command      = communication::LOGIN;
    packet->seqn         = 1;
    packet->total_size   = sizeof(send_username);
    packet->length       = sizeof(send_username);
    packet->_payload     = send_username;
    //
    transmitter->sendPackage(packet);
    delete(transmitter);

    //start threads
    //
    //
};

void *Client::handleUserInput(communication::Command command, std::string filename)
{

    switch (command)
    {
        case communication::UPLOAD:
            if(!filename.empty())
            {
                namespace fs = std::filesystem;
                fs::path f{filename};
                if (!fs::exists(f))
                    std::cout << "ERROR: file not present";

                //sendfile
            }
            break;
        case communication::DOWNLOAD:
            break;
        case communication::DELETE:
            if(!filename.empty())
            {
                namespace fs = std::filesystem;
                fs::path f{filename};
                if (!fs::exists(f))
                    std::cout << "ERROR: file not present";
            }
            break;
        case communication::GET_SYNC_DIR:
            break;
        case communication::LIST_SERVER:
            break;
        case communication::LIST_CLIENT:
        {
            namespace fs = std::filesystem;
            std::string path = "/sync_dir";
            for (const auto &entry: fs::directory_iterator(path))
                std::cout << entry.path() << std::endl;
        }
            break;
        default:
            break;
    }

};
void *Client::handleServerOutput()
{
    //TODO
void *Client::keepAlive(void* arg)
{
    //TODO
};