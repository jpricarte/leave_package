#include <iostream>
#include <thread>
#include <cstring>

#include <sys/socket.h>
#include <signal.h>
#include <netinet/in.h>

#include "user.h"
#include "communication.h"
#include "RequestHandler.h"

const int PORT = 4001;

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


void communicationHandler(communication::Transmitter* transmitter, user::UserManager* user_manager)
{
    std::string username;
    user::User* user;
    // Aqui recebe a primeira mensagem do cliente, recebendo o Username associado
    try {
        auto user_info = transmitter->receivePacket();
        if (user_info.command != communication::LOGIN)
        {
            transmitter->sendPacket(communication::LOGIN_FAIL);
            delete transmitter;
            return;
        }
        username = std::string(user_info._payload);
    } catch (communication::SocketReadError& e) {
        transmitter->sendPacket(communication::LOGIN_FAIL);
        cerr << e.what() << endl;
        delete transmitter;
        return;
    }

    user = user_manager->findOrCreateUser(username);

    try {
        user->tryConnect(transmitter->getSocketfd(), transmitter);
    } catch (user::TooManyConnections& e) {
        transmitter->sendPacket(communication::LOGIN_FAIL);
        cerr << e.what() << endl;
        delete transmitter;
        return;
    }

    cout << username << " logged successfully" << endl;
    try {
        transmitter->sendPacket(communication::SUCCESS);
    } catch (communication::SocketWriteError& e) {
        cerr << e.what() << endl;
    }

    auto* request_handler = new RequestHandler(transmitter, user);
    auto income = thread(&RequestHandler::handleIncome, request_handler);
    auto outcome = thread(&RequestHandler::syncWithOtherDevice, request_handler);

    income.join();
    outcome.join();
    cout << "I'll miss " << username << endl;

    user->disconnect(transmitter->getSocketfd());
    close(transmitter->getSocketfd());
    delete transmitter;
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
        auto* client_addr = new sockaddr_in;
        int main_socket = accept(welcome_socket, (struct sockaddr *) client_addr, &cli_len);
        if (main_socket == -1)
        {
            cerr << "shutdown or ERROR on accept" << endl;
            continue;
        }

        auto* transmitter = new communication::Transmitter(client_addr, main_socket);

        threads_pool.push_back(new thread(&communicationHandler, transmitter, user_manager) );
    }
    return 0;
}