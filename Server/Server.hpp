#ifndef SERVER_HPP
#define SERVER_HPP

#include "SHARED.hpp"


#define UDP_PORT 8081

#define ERR_USERNAME_STR "ERR: Username already exists."
#define ERR_ROLE_STR "ERR: Invalid role."

#define START_STR "start"

std::set<int> assigned_ports;
int BASE_PORT = 5000;

using namespace std;

class Server
{
private:
    struct sockaddr_in broadcast_addr;

    int port_cntr = 0;

    int server_fd;
    int stp_port;

    int udp_socket;
    int udp_port = UDP_PORT;

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

    // _____________ Check Client Info _____________
    int HasUniqueUsername(Client_info new_client);
    int HasValidRole(Client_info new_client);
    int checkClientInfo(Client_info new_client);
    // _____________ Check Client Info _____________

    // addNewClient function
    // handle new client functoin
public:
    Server(int port) : stp_port(port), server_fd(-1) {}
    ~Server();

    void start() {
        // // ایجاد سوکت TCP (سرور)
        // server_fd = create_socket(false, false);
        // bind_socket(server_fd, stp_port, false);
        // if (listen(server_fd, 5) < 0) {
        //     my_print("listen failed\n");
        //     exit(EXIT_FAILURE);
        // }
    
        // // ایجاد سوکت UDP فقط برای ارسال Broadcast
        // udp_socket = create_socket(true, true);
    
        // my_print("Server listening on port ");
        // my_print(std::to_string(stp_port).c_str());
        // my_print("\n");
    
        // // راه‌اندازی پردازش TCP و UDP
        // startServer();
    }
};



#endif // GAME_MANAGER_H