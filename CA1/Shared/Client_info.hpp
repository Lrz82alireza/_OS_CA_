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
    std::shared_ptr<User> user = nullptr;

    Client_info(/* args */) {
    }
    ~Client_info() {}

    std::string getUserName();
};


#endif
