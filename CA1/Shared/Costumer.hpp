#ifndef COSTUMER_HPP
#define COSTUMER_HPP

#include "User.hpp"

class Costumer : public User
{
private:
    /* data */
public:
    Costumer(const User& user) {
        this->username = user.username;
        this->password = user.password;
        this->role = user.role;
    }
    ~Costumer() {}
};


#endif