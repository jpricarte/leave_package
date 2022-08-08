//
// Created by santi on 02/08/2022.
//

#ifndef LEAVE_PACKAGE_CLIENT_H
#define LEAVE_PACKAGE_CLIENT_H



#ifndef CLIENT_H
#define CLIENT_H

# include <sys/socket.h>
#include <netinet/in.h>
#include <ostream>
#include <semaphore>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <atomic>
#include <exception>

#include "communication.h"
#include "monitor.h"



class Client
{
    //attributes
private:


    static std::string username;                 // client usernme
    std::string server_ip;                       // server ip
    int         port;                            // server port
    static int  server_socket;                   // Socket for remote server communication
    struct      sockaddr_in *server_address;     // Server socket address

    static      pthread_t input_handler_thread;  // Thread to listen for new incoming server messages
    static      pthread_t output_handler_thread;     // Thread to keep the client "alive" by sending periodic messages to server

    static      Monitor monitor;                 // Monitor that controls the sending of data through the socket
    static std::atomic<bool> stop;
    //methods
public:
    //Constructor
    Client(std::string username, std::string server_ip, std::string port);
    //Destructor
    ~Client();

    void setupConnection();
    //temp
    static communication::Command parseCommand(const std::string s);
    //

private:

    static void *handleUserInput(std::string command, std::string filename);
    static void *handleServerOutput();
};

#endif //LEAVE_PACKAGE_CLIENT_H