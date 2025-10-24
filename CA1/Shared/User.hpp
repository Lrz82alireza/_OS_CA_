#ifndef USER_HPP
#define USER_HPP

#include <string>

class User
{
private:
public:
    std::string username;
    std::string password;
    int role;

    User() = default;
    User(std::string username, std::string password, int role)
        : username(username), password(password), role(role){

    } 
    ~User() {}
};


#endif