//
// Created by santi on 02/08/2022.
//

#include "../include/client.h"
#include "../include/communication.h"

using namespace std;
std::atomic<bool> Client::stop_issued;
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
        printf("ERROR: unable to create socket");
        exit(0);
    }
    // Fill server socket address
    server_address.sin_family = AF_INET;
    server_address.sin_port   = htons(port);
    server_address.sin_addr.s_addr = inet_addr(server_ip.c_str());

    // Try to connect to server
    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("ERROR: unable to connect to server");
        exit(0);
    }
    //login packet
    int Packet_size = sizeof(communication::Packet);
    char * send_username = const_cast<char*>(username.c_str());
    communication::Packet* data = (communication::Packet*)malloc(Packet_size);
    bzero((void*)data, Packet_size);
    data->command      = communication::LOGIN;
    data->seqn         = 1;
    data->total_size   = sizeof(send_username);
    data->length       = sizeof(send_username);
    data->_payload     =send_username;
    communication::Transmitter::sendPackage(data); //send packet deveria conter parametros pra onde enviar
    //idealmente sendpckg(socket, detalhes do pacote)

};

void *Client::handleUserInput(void* arg)
{
    //TODO
};
void *Client::handleServerOutput()
{
    //TODO
};
void *Client::keepAlive(void* arg)
{
    //TODO
};