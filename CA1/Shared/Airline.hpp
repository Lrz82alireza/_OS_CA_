#ifndef AIRLINE_HPP
#define AIRLINE_HPP

#include "Client_info.hpp"

class Airline : public Client_info
{
private:
    /* data */
public:
    Airline(/* args */) {}
    Airline(const Client_info& client) {
        this->username = client.username;
        this->password = client.password;
        this->role = client.role;
        this->port = client.port;
        this->client_fd = client.client_fd;
        this->isLoggedIn = client.isLoggedIn;
    }
    ~Airline() {}
};


#endif