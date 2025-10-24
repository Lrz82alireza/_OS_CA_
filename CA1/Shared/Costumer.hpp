#ifndef COSTUMER_HPP
#define COSTUMER_HPP

#include "Client_info.hpp"

class Costumer : public Client_info
{
private:
    /* data */
public:
    Costumer(/* args */) {}
    Costumer(const Client_info& client) {
        this->username = client.username;
        this->password = client.password;
        this->role = client.role;
        this->port = client.port;
        this->client_fd = client.client_fd;
        this->isLoggedIn = client.isLoggedIn;
    }
    ~Costumer() {}
};


#endif