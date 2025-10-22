#ifndef CLIENT_INFO_HPP
#define CLIENT_INFO_HPP

class Client_info
{
private:
    /* data */
public:
    char username[50];
    char password[50];
    char role[20];
    int port;
    int client_fd;

    Client_info(/* args */);
    ~Client_info();
};

Client_info::Client_info(/* args */)
{
}

Client_info::~Client_info()
{
}


#endif
