#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#include "SHARED.hpp"

#define UDP_PORT 8081

using namespace std;

class Client
{
private:
    int tcp_sock = -1;
    int udp_sock = -1;
    int port;
    const char* server_ip;
public:
    Client(int port, const char* server_ip = "127.0.0.1")
        : port(port), server_ip(server_ip) {
            
    }
    ~Client() {
        if (tcp_sock != -1) close(tcp_sock);
    }

    // _____________ CONNECTION FUNC. _____________
    // تابع برای ایجاد و اتصال سوکت TCP به سرور
    int connectToTcpServer(const char* server_ip, int port);
    // تابع برای دریافت پورت جدید از سرور
    int receiveNewPort(int sock);
    // تابع برای بستن اتصال TCP اولیه
    void closeTcpConnection(int sock);
    // تابع برای ارسال اطلاعات کاربر به سرور
    void sendUserInfo(int sock, const char* username, const char* role);
    // تابع برای ایجاد و تنظیم سوکت UDP
    int setupUdpSocket(int udp_port);
    // تابع اصلی برای اتصال به سرور
    void connectToServer();    
    // _____________ CONNECTION FUNC. _____________   

    // تابع برای آماده‌سازی مجموعه
    void prepareFdSet(fd_set& read_fds, int tcp_sock, int udp_sock);
    void handleUserInput(int tcp_sock);
    bool handleTcpMessage(int tcp_sock);
    void handleUdpMessage(int udp_sock);

    void startClient();
};


#endif // CLIENT_HPP