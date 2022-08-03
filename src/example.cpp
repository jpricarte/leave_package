#include <iostream>
#include <thread>
#include "../include/user.h"

/* *
 * Código de exemplo: tenta inserir 2 vezes um usuário na lista de usuários ativos
 * */

using namespace std;
int main() {

    user::UserManager user_manager {};
    user::User my_user("jricarte");
    user::User other_user("vcoimbra");
    try {
        auto t1 = thread(&user::UserManager::userLogin, &user_manager, my_user);
        auto t2 = thread(&user::UserManager::userLogin, &user_manager, other_user);
        t1.join();
        t2.join();
    }
//  Não está funcionando da forma que devia, se estoura erro na thread worker,
//  não será tratada
    catch (const SemaphoreOverused &e) {
        cout << e.what() << endl;
    }

//  Não é multi-thread, é só pra mostrar os métodos
    auto r1 = user_manager.isUserActive(my_user);
    auto r2 = user_manager.isUserActive(other_user);

    cout << "User 1: " << r1 << '\t';
    cout << "User 2: " << r2 << endl;

    try {
        auto t1 = thread(&user::UserManager::userLogout, &user_manager, my_user);
        auto t2 = thread(&user::UserManager::userLogout, &user_manager, other_user);
        t1.join();
        t2.join();
    }
//  Não está funcionando da forma que devia, se estoura erro na thread worker,
//  não será tratada
    catch (const SemaphoreOverused &e) {
        cout << e.what() << endl;
    }

    r1 = user_manager.isUserActive(my_user);
    r2 = user_manager.isUserActive(other_user);

    cout << "User 1: " << r1 << '\t';
    cout << "User 2: " << r2 << endl;

    return 0;
}
