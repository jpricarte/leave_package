#include <iostream>
#include <thread>
#include "user.h"

using namespace std;
int main() {
//    Primeiro, configuramos o servidor TCP e abrimos ele para conexão

//    Depois, abrimos um looping para experar conexões
    while (true) {
//        Ao receber uma nova conexão, cria um processo para lidar com o programa
        auto proccess_id = 0; // Obviamente vai ser algo mais complexo que isso
        if (proccess_id == 0) {
            cout << "new connection established with " << "[CLIENT IP]" << endl;
//        Começa associando a conexão a um usuário (caso esteja disponível)
//        Espera comandos, e executa da forma devida
//        Termina finalizando a conexão e matando o processo filho
        }
    }
    return 0;
}
