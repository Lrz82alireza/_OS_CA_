#ifndef SERVER_HPP
#define SERVER_HPP

#include "SHARED.hpp"
#include "SeatMap.hpp"

#define UDP_PORT_AIRLINE 8081
#define UDP_PORT_COSTUMER 8082
#define ROLE_AIRLINE 1
#define ROLE_COSTUMER 2

#define ERR_USERNAME_STR "ERR: Username already exists."
#define ERR_ROLE_STR "ERR: Invalid role."

#define SEATS_COLUMN_MAX_SIZE 26
#define SEATS_ROW_MAX_SIZE 100

#define START_STR "start"

std::set<int> assigned_ports;
int BASE_PORT = 5000;

using namespace std;

struct UdpChannel {
    int fd;
    int port;
    sockaddr_in addr;
};

struct UdpSocket {
    UdpChannel airLine;
    UdpChannel customer;
};


class Server
{
private:
    UdpSocket udpSocket;

    int port_cntr = 0;

    int server_fd;
    int stp_port;

    std::vector<Client_info*> clients;

    // _____________ CONNECTION FUNC. _____________
    
    // تابع برای دریافت پورت اختصاص داده‌شده به کلاینت
    int getAssignedPort(int client_fd);
    // تابع برای پذیرش اتصال جدید از کلاینت
    int acceptNewClient(int server_fd); 
    // تابع برای اختصاص پورت جدید به کلاینت
    int assignNewPort();
    // تابع برای ارسال پورت جدید به کلاینت
    void sendNewPortToClient(int client_fd, int new_port);
    // تابع برای انتظار و پذیرش اتصال مجدد کلاینت به پورت جدید
    int waitForClientOnNewPort(int new_server_fd);
    // تابع برای دریافت اطلاعات از کلاینت
    bool receiveClientInfo(int client_fd, Client_info &new_client);

    // _____________ توابع کمکی _____________
    void prepareFdSetForServer(fd_set& read_fds, int& max_fd);
    void handleNewConnections(fd_set& read_fds);
    void handleClientMessages(fd_set& read_fds);
    void handleUdpBroadcast(int socket_fd, const sockaddr_in& addr);
    void handleKeyboardInput(fd_set &read_fds);

    // _____________ Check Client Info _____________
    int HasUniqueUsername(Client_info new_client);
    int HasValidRole(Client_info new_client);
    int checkClientInfo(Client_info new_client);
    // _____________ Check Client Info _____________

    int addNewClient(Client_info new_client);
    void handleNewClient(int server_fd);
public:
    Server(int port) : stp_port(port), server_fd(-1) {}
    ~Server() {}

    void startServer();

    void start() {
        // ایجاد سوکت TCP (سرور)
        server_fd = create_socket(false, false);
        bind_socket(server_fd, stp_port, false);
        if (listen(server_fd, 5) < 0) {
            my_print("listen failed\n");
            exit(EXIT_FAILURE);
        }
    
        // ایجاد سوکت UDP فقط برای ارسال Broadcast
        udpSocket.airLine.fd = create_socket(true, true);
        udpSocket.customer.fd = create_socket(true, true);
        udpSocket.airLine.port = UDP_PORT_AIRLINE;
        udpSocket.customer.port = UDP_PORT_COSTUMER;
    
        my_print("Server listening on port ");
        my_print(std::to_string(stp_port).c_str());
        my_print("\n");
    
        // راه‌اندازی پردازش TCP و UDP
        startServer();
    }
};



#endif // SERVER_HPP