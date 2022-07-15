#include <iostream>
#include <thread>
#include "user.h"

/* *
 * Código de exemplo: tenta inserir 2 vezes um usuário na lista de usuários ativos
 * */

using namespace std;
int main() {

    user::UserManager user_manager {};
    string my_username = "jpricarte";
    user::User my_user(my_username);

    try {
        std::thread t1(&user::UserManager::insertUser, &user_manager, my_user);
        std::thread t2(&user::UserManager::insertUser, &user_manager, my_user);

        t1.join();
        t2.join();
    } catch (SemaphoreOverused &e) {
        cout << e.what() << endl;
    }

    bool is_active = user_manager.isUserActive(my_user);


    cout << is_active << endl;

    return 0;
}
