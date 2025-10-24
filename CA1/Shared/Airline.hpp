#ifndef AIRLINE_HPP
#define AIRLINE_HPP

#include "User.hpp"

class Airline : public User
{
private:
    /* data */
public:
    Airline(const User& User) {
        this->username = User.username;
        this->password = User.password;
        this->role = User.role;
    }
    ~Airline() {}
};


#endif