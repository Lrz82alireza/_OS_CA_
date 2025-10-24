#ifndef CUSTOMER_HPP
#define CUSTOMER_HPP

#include "User.hpp"

class Customer : public User
{
private:
    /* data */
public:
    Customer(const User& user) {
        this->username = user.username;
        this->password = user.password;
        this->role = user.role;
    }
    ~Customer() {}
};


#endif