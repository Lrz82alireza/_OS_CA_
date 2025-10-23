#ifndef CLIENT_INFO_HPP
#define CLIENT_INFO_HPP

class Client_info
{
private:
    /* data */
public:
    std::string username;
    std::string password;
    int role;
    int port = -1;
    int client_fd;
    bool isLoggedIn;

    Client_info(/* args */) {
        this->isLoggedIn = false;
    }
    ~Client_info() {}
};


#endif
