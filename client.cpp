//
// Created by santi on 02/08/2022.
//

#include <iostream>
#include <thread>
#include <cstring>
#include <cstdio>

#include <sys/socket.h>
#include <netinet/in.h>

#include "client.h"
#include "communication.h"
#include "commandHandler.h"


using namespace std;
using namespace communication;
std::string Client::username;
int Client::server_socket;

pthread_t Client::input_handler_thread;
pthread_t Client::output_handler_thread;
static std::atomic<bool> stop;
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
    auto* transmitter = new Transmitter(server_address, server_socket);
    //login packet
    //char * send_username = const_cast<char*>(username.c_str());
    Packet packet{
            LOGIN,
            0,
            (unsigned long int)username.size(),
            (unsigned int) username.size(),
            const_cast<char *>(username.c_str())
    };

    //packet->command      = LOGIN;
    //packet->seqn         = 0;
    //packet->total_size   = sizeof(send_username);
    //packet->length       = sizeof(send_username);
    //packet->_payload     = send_username;
    //
    try{
    transmitter->sendPackage(packet);
    } catch (SocketWriteError& e) {
    cerr << e.what() << endl;
    }
    delete(transmitter);
    stop = false;
    //TODO start threads
    //
    //
};
//temp
Command parseCommand(const std::string s) {
    if (s == "exit")
        return EXIT;
    if (s == "list_client")
        return LIST_CLIENT;
    if (s == "list_server")
        return LIST_SERVER;
    if (s == "get_sync_dir")
        return GET_SYNC_DIR;
    if (s == "upload")
        return UPLOAD;
    if (s == "download")
        return DOWNLOAD;
    if (s == "delete")
        return DELETE;
    else
        return NOP;
}


//
void *Client::handleUserInput(std::string command, std::string filename)
{
    Command command_parsed = parseCommand(command);
    unsigned int bytes_read = -1;
    char file_buffer[256];
    unsigned long int size;
    while(!stop) {

        switch (command_parsed) {
            case UPLOAD:
                if (!filename.empty()) {
                    namespace fs = filesystem;
                    fs::path f{filename};
                    if (!fs::exists(f))
                        std::cout << "ERROR: file not present";
                        exit(0);

                    char * fn = const_cast<char*>(filename.c_str());
                    FILE *fd = fopen(fn, "rb");

                    fseek(fd, 0, SEEK_END);
                    size = ftell(fd); // get file size
                    fseek(fd, 0, SEEK_SET);//returns pointer to beginning of file

                    //send file header
                    //should count the header in the total size? (not included as of now)
                    Packet header_pkt {
                            communication::UPLOAD,
                            1,
                            size,
                            (unsigned int) filename.size(),
                            (char*) filename.c_str()
                    };
                    //transmitter->sendpacket(header_pkt);
                    //send file
                    unsigned int sqn = 2;
                    while (!feof(fd)) {
                        if ((bytes_read = fread(&file_buffer, 1, 256, fd)) > 0) {
                            Packet data_pkt{
                                    communication::UPLOAD,
                                    sqn,
                                    size,
                                    bytes_read,
                                    file_buffer
                            };
                            //transmitter->sendpacket(data_pkt);
                            sqn++;
                        }
                        else
                            break;

                    }
                    fclose(fd);

                }
                break;
            case DOWNLOAD:
                break;
            case DELETE: {

                Packet packet {
                        communication::DELETE,
                        1,
                        (unsigned long int) filename.size(),
                        (unsigned int) filename.size(),
                        (char*) filename.c_str()
                };

                char * fn = const_cast<char*>(filename.c_str());
                int removed = remove(fn);
                if (removed != 0) {
                    std::cout << "ERROR: file was not present to begin with";
                }

            }
                break;
            case GET_SYNC_DIR:
                break;
            case LIST_SERVER:
                break;
            case LIST_CLIENT: {
                //from https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
                namespace fs = filesystem;
                std::string path = "/sync_dir";
                for (const auto &entry: fs::directory_iterator(path))
                    std::cout << entry.path() << std::endl;
            }
            case EXIT:
                //send pkt to server
                //close comms
                stop = true;
                close(server_socket);
                break;
            default:
                break;
        }
    }
    //TODO finish threads

};
void *Client::handleServerOutput() {
    //TODO
}