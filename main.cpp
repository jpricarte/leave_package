#include <iostream>
#include <thread>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>

#include "user.h"
#include "communication.h"

const int PORT = 4001;

using namespace std;

void communicationHandler(communication::Transmitter* transmitter, user::UserManager* user_manager)
{
    user::User* user = nullptr;
    std::string username;
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
        cout << username << endl;
    } catch (SocketReadError& e) {
        transmitter->sendPackage(communication::LOGIN_FAIL);
        cerr << e.what() << endl;
        delete transmitter;
        return;
    }

    int tries = 0;
    do {
        try {
            user = user_manager->createUser(username);
            // TODO: FAZER TODA A MÃO DE SALVAR CONEXÃO no usuário
        } catch (SemaphoreOverused& e) {
            cerr << username << ": " << e.what() << endl;
            tries++;
        }
    } while (tries < 3);

    if (tries == 4) {
        cerr << username << ": " << "Server overload, finishing connection" << endl;
        try {
            transmitter->sendPackage(communication::LOGIN_FAIL);
        } catch (SocketWriteError& e) {
            cerr << username << ": " << e.what() << endl;
        }
        delete transmitter;
        return;
    }

    cout << user->getUsername() << " logged successfully" << endl;
    try {
        transmitter->sendPackage(communication::SUCCESS);
    } catch (SocketWriteError& e) {
        cerr << e.what() << endl;
    }

//    TODO: handler de commandos recebidos
    communication::Command last_command = communication::NOP;
    while(last_command != communication::EXIT)
    {
        try {
            auto package = transmitter->receivePackage();
            last_command = package.command;
        } catch (SocketReadError& e) {
            cerr << e.what() << endl;
        }
    }

    cout << "I'll miss " << username << endl;
//    TODO: FAZER LOGOUT ANTES DE SAIR
    delete transmitter;
}

int main() {
    user::UserManager user_manager{};

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
    // Depois, abrimos um looping para esperar conexões
    int c = 0;
    while (c < 5){ //TODO: transformar em loop infinito
        socklen_t cli_len = sizeof(struct sockaddr_in);
        struct sockaddr_in* client_addr = new sockaddr_in;
        int new_sockfd = accept(sockfd, (struct sockaddr *) client_addr, &cli_len);
        if (new_sockfd == -1)
        {
            cerr << "ERROR on accept" << endl;
            continue;
        }

        auto* transmitter = new communication::Transmitter(client_addr, new_sockfd);
        communicationHandler(transmitter, &user_manager);

        c++; // conexão estabelecida TODO: tirar isso quando arrumar o loop

        // Começa associando a conexão a um usuário (caso esteja disponível)
        // TODO: trocar uma primeira mensagem recebendo o nome do usuario

        // Ao receber uma nova conexão, cria uma nova thread para lidar com o programa
        cout << "new connection established with " << client_addr->sin_addr.s_addr << endl;

        // Espera comandos, e executa da forma devida
        // Termina finalizando a conexão e matando o processo filho
    }

    close(sockfd);
    return 0;
}
/*  Comunicação
 *  PASSO 1 - Cliente envia username para ceonectar <- USARIO
 *
 *  Passo 2 - Servidor salvar as informações (IP) da sincronização
 *
 *  Passo 3 - Servidor envia um socket(TCP) com a listagem dos arquivos dentro do proprio (DOWNLOAD)
 *
 *  Passo 4 - Usuario faz upload no servidor.
 *
 *  Passo 5 - Servidor faz gerenciar o arquivo
 *
 *  Passo 6 - servidor verifica se a outras conexão e sincroniza os arquivos
 *
 */