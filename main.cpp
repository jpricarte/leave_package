#include <iostream>
#include <thread>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>

#include "user.h"
#include "communication.h"

const int PORT = 4002;

using namespace std;

void communicationHandler(communication::Transmitter& transmitter, user::User& user)
{
    cout << "not implemented" << endl;
}

int main() {

    user::UserManager user_manager{};

    // Primeiro, configuramos o servidor TCP e abrimos ele para conexão
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        cerr << "ERROR opening socket" << endl;

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_addr.sin_zero), 8);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        cerr << "ERROR on binding" << endl;

    listen(sockfd, 5);

    // Depois, abrimos um looping para esperar conexões
    int c = 0;
    while (c < 5){ //TODO: transformar em loop funcional
        socklen_t cli_len = sizeof(struct sockaddr_in);
        struct sockaddr_in client_addr{};
        int new_sockfd = accept(sockfd, (struct sockaddr *) &client_addr, &cli_len);
        if (new_sockfd == -1)
        {
            cerr << "ERROR on accept" << endl;
            continue;
        }

        auto transmitter = new communication::Transmitter(&client_addr, new_sockfd);
        // Aqui recebe a primeira mensagem do cliente, recebendo o User associado
        try {
            communication::Packet p = transmitter->receivePackage();
            cout << p.length << endl;
            cout << p._payload << endl;
        } catch (SocketReadError e) {
            cerr << e.what() << endl;
        }
//        string new_username(userInfo._payload,0, userInfo.length);
//        cout << new_username << endl;
        delete transmitter;
//        try {
//            transmitter->receivePackage<communication::CommandPacket>(&userInfo);
//            if (userInfo.command == communication::LOGIN) {
//                string new_username(userInfo._payload);
//                user::User new_user(new_username);
//                cout << new_username << endl;
////                auto t = thread(&communicationHandler, transmitter, new_user);
//            }
//        } catch (SocketReadError e) {
//            cerr << e.what() << endl;
//        }

        c++; // conexão estabelecida TODO: tirar isso quando arrumar o loop

        // Começa associando a conexão a um usuário (caso esteja disponível)
        // TODO: trocar uma primeira mensagem recebendo o nome do usuario

        // Ao receber uma nova conexão, cria uma nova thread para lidar com o programa
        cout << "new connection established with " << client_addr.sin_addr.s_addr << endl;

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