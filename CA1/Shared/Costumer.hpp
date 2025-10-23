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
        this->role = client.role;
        this->username = client.username;
        this->password = client.password;
    }
    ~Costumer() {}
};


#endif