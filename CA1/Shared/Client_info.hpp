#ifndef CLIENT_INFO_HPP
#define CLIENT_INFO_HPP

#include "User.hpp"
#include <memory>

class Client_info
{
private:
    /* data */
public:
    int port = -1;
    int client_fd;
    bool isLoggedIn;
    std::shared_ptr<User> user = nullptr;

    Client_info(/* args */) {
        this->isLoggedIn = false;
    }
    ~Client_info() {}
};


#endif
