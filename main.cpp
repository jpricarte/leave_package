#include <iostream>
#include <thread>
#include <cstring>

#include <sys/socket.h>
#include <signal.h>
#include <netinet/in.h>

#include "user.h"
#include "communication.h"
#include "RequestHandler.h"

using namespace std;

std::vector<thread*> threads_pool{};
int welcome_socket=0;
bool accepting_conections = true;

int handleSigInt()
{
    close(welcome_socket);
    cout << "closing" << endl;
    for (std::thread* t: threads_pool) {
        if (t->joinable()) {
            t->join();
        }
    }
    accepting_conections = false;
    return 0;
}


void communicationHandler(communication::Transmitter* command_transmitter,
                          communication::Transmitter* update_transmitter,
                          user::UserManager* user_manager)
{
    std::string username;
    user::User* user;
    // Aqui recebe a primeira mensagem do cliente, recebendo o Username associado
    try {
        auto user_info = command_transmitter->receivePacket();
        if (user_info.command != communication::LOGIN)
        {
            command_transmitter->sendPacket(communication::LOGIN_FAIL);
            delete update_transmitter;
            delete command_transmitter;
            return;
        }
        username = std::string(user_info._payload);
    } catch (communication::SocketReadError& e) {
        command_transmitter->sendPacket(communication::LOGIN_FAIL);
        cerr << e.what() << endl;
        delete command_transmitter;
        delete update_transmitter;
        return;
    }

    user = user_manager->findOrCreateUser(username);
    auto update_handler = new UpdateHandler(update_transmitter, user->getFileManager());
    try {
        user->tryConnect(command_transmitter->getSocketfd(), command_transmitter, update_handler);
    } catch (user::TooManyConnections& e) {
        command_transmitter->sendPacket(communication::LOGIN_FAIL);
        cerr << e.what() << endl;
        delete command_transmitter;
        delete update_transmitter;
        return;
    }

    cout << username << " logged successfully" << endl;
    try {
        command_transmitter->sendPacket(communication::SUCCESS);
    } catch (communication::SocketWriteError& e) {
        cerr << e.what() << endl;
    }

    auto* request_handler = new RequestHandler(command_transmitter, user);
    auto income = thread(&RequestHandler::handleIncome, request_handler);

    income.join();
    cout << "I'll miss " << username << endl;

    user->disconnect(command_transmitter->getSocketfd());
    close(command_transmitter->getSocketfd());
    close(update_transmitter->getSocketfd());
    delete command_transmitter;
    delete update_handler;
    delete update_transmitter;
}


int main(int argc, char* argv[]) {

    if (argc != 2)
    {
        cerr << "usage:" << argv[0] << " <port>" << endl;
        return -1;
    }

    signal(SIGINT, reinterpret_cast<__sighandler_t>(handleSigInt));
    signal(SIGTERM, reinterpret_cast<__sighandler_t>(handleSigInt));

    auto* user_manager = new user::UserManager();

    // Primeiro, configuramos o servidor TCP e abrimos ele para conexão
    welcome_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (welcome_socket < 0)
    {
        cerr << "ERROR opening socket" << endl;
        return -1;
    }

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_addr.sin_zero), 8);

    if (bind(welcome_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        cerr << "ERROR on binding" << endl;
        return -2;
    }

    listen(welcome_socket, 5);

    cout << "accepting connections" << endl;
    // Depois, abrimos um looping para esperar conexões
    while (accepting_conections) {
        socklen_t cli_len = sizeof(struct sockaddr_in);
        auto* main_client_addr = new sockaddr_in;
        int main_socket = accept(welcome_socket, (struct sockaddr *) main_client_addr, &cli_len);
        if (main_socket == -1)
        {
            cerr << "shutdown or ERROR on accept" << endl;
            continue;
        }

        auto* update_client_addr = new sockaddr_in;
        int update_socket = accept(welcome_socket, (struct sockaddr *) update_client_addr, &cli_len);
        if (update_socket == -1)
        {
            cerr << "shutdown or ERROR on accept" << endl;
            close(main_socket);
            continue;
        }

        auto* main_transmitter = new communication::Transmitter(main_client_addr, main_socket);
        auto* update_transmitter = new communication::Transmitter(update_client_addr, update_socket);


        threads_pool.push_back(new thread(&communicationHandler, main_transmitter, update_transmitter, user_manager));
    }
    return 0;
}