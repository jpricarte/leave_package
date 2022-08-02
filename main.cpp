#include <iostream>
#include <thread>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>

#include "user.h"
#include "communication.h"
#include "commandHandler.h"

const int PORT = 4001;

using namespace std;

std::mutex threads_manager_mutex;
std::vector<thread*> threads_manager{};


void communicationHandler(communication::Transmitter* transmitter, user::UserManager* user_manager)
{
    std::string username;
    user::User* user;
    // Aqui recebe a primeira mensagem do cliente, recebendo o Username associado
    try {
        auto user_info = transmitter->receivePackage();
        if (user_info.command != communication::LOGIN)
        {
            transmitter->sendPackage(communication::LOGIN_FAIL);
            delete transmitter;
            return;
        }
        username = std::string(user_info._payload);
    } catch (communication::SocketReadError& e) {
        transmitter->sendPackage(communication::LOGIN_FAIL);
        cerr << e.what() << endl;
        delete transmitter;
        return;
    }

    user = user_manager->findOrCreateUser(username);

    try {
        user->tryConnect(transmitter->getSocketfd(), transmitter->getClientAddr());
    } catch (user::TooManyConnections& e) {
        transmitter->sendPackage(communication::LOGIN_FAIL);
        cerr << e.what() << endl;
        delete transmitter;
        return;
    }

    cout << username << " logged successfully" << endl;
    try {
        transmitter->sendPackage(communication::SUCCESS);
    } catch (communication::SocketWriteError& e) {
        cerr << e.what() << endl;
    }

//    TODO: handler de commandos recebidos
//    IMPORTANTE: NÃO FECHAR SOCKET NAS THREADS AUXILIARES
    auto* command_handler = new commandHandler(transmitter);
    auto income = thread(&commandHandler::handleIncome, command_handler);

    income.join();
    cout << "I'll miss " << username << endl;
//    TODO: FAZER LOGOUT ANTES DE SAIR
    user->disconnect(transmitter->getSocketfd());
    delete transmitter;
}

std::mutex connection_mutex;
void readCommand(bool* accepting_conections)
{
    string command;
    do {
        connection_mutex.unlock();
        cin >> command;
        if (command == "exit")
        {
            connection_mutex.lock();
            *accepting_conections = false;
            connection_mutex.unlock();
        }
        connection_mutex.lock();
    } while (*accepting_conections);
}

int main() {

    auto* user_manager = new user::UserManager();

    // Primeiro, configuramos o servidor TCP e abrimos ele para conexão
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        cerr << "ERROR opening socket" << endl;
        return -1;
    }

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_addr.sin_zero), 8);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        cerr << "ERROR on binding" << endl;
        return -2;
    }

    listen(sockfd, 5);

    cout << "accepting connections" << endl;
    bool accepting_conections = true;
    auto th_control = thread(&readCommand, &accepting_conections);
    // Depois, abrimos um looping para esperar conexões
    connection_mutex.lock();
    while (accepting_conections) {
        connection_mutex.unlock();
        socklen_t cli_len = sizeof(struct sockaddr_in);
        auto* client_addr = new sockaddr_in;
        int new_sockfd = accept(sockfd, (struct sockaddr *) client_addr, &cli_len);
        if (new_sockfd == -1)
        {
            cerr << "ERROR on accept" << endl;
            continue;
        }

        auto* transmitter = new communication::Transmitter(client_addr, new_sockfd);

        threads_manager_mutex.lock();
            threads_manager.push_back( new thread(&communicationHandler, transmitter, user_manager) );
        threads_manager_mutex.unlock();

        connection_mutex.lock();
    }
    close(sockfd);

    for (std::thread* t: threads_manager) {
        if (t->joinable()) {
            t->join();
        }
    }
    th_control.join();

    return 0;
}
