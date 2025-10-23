#ifndef SERVER_HPP
#define SERVER_HPP

#include "SHARED.hpp"
#include "SeatMap.hpp"
#include "FlightManager.hpp"

#define BASE_PORT 5000

#define UDP_PORT_AIRLINE 8081
#define UDP_PORT_COSTUMER 8082

#define ROLE_AIRLINE_STR "AIRLINE"
#define ROLE_AIRLINE 1
#define ROLE_COSTUMER_STR "COSTUMER"
#define ROLE_COSTUMER 2

// Response Messages
#define APPROVED_REGISTER_STR "REGISTERED OK"
#define ERR_USERNAME_STR "ERROR UsernameAlreadyExists"
#define ERR_ROLE_STR "ERR: Invalid role."

#define SEATS_COLUMN_MAX_SIZE 26
#define SEATS_ROW_MAX_SIZE 100

#define START_STR "start"

std::set<int> assigned_ports;

using namespace std;

class Server
{
private:
    UdpSocket udpSocket;

    int server_fd;
    int stp_port;

    // bool start_flag = false;
    bool start_flag = true;

    std::vector<std::shared_ptr<Client_info>>clients;
    std::vector<std::shared_ptr<Airline>> airlines;
    std::vector<std::shared_ptr<Costumer>> costumers;

    std::shared_ptr<FlightManager> flightManager = nullptr;

    map<int, function<void(shared_ptr<Client_info>, const string&)>> commandHandlers;

    // _____________ DISPATCHER FUNC. _____________
    void handleRegister(shared_ptr<Client_info> client, const string& content);
    void handleLogin(shared_ptr<Client_info> client, const string& content);


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
    void handleUdpBroadcast(int socket_fd, const sockaddr_in& addr);
    void handleKeyboardInput(fd_set &read_fds);

    void handleClientMessages(fd_set& read_fds);
    void handleLoggedOutMessages(shared_ptr<Client_info> client_info, const char* buffer, int len);
    void handleLoggedInMessages(shared_ptr<Client_info> client_info, const char* buffer, int len);

    // _____________ Check Client Info _____________
    int HasUniqueUsername(string username);
    int HasValidRole(string role);
    int checkClientInfo(Client_info new_client, string username, string role);
    // _____________ Check Client Info _____________

    int registerClient(shared_ptr<Client_info> new_client, string username, string password, string role);
    void handleNewClient(int server_fd);
public:
    Server(int port) : stp_port(port), server_fd(-1) {
        commandHandlers[REGISTER_N] = std::bind(&Server::handleRegister, this, std::placeholders::_1, std::placeholders::_2);
        commandHandlers[LOGIN_N] = std::bind(&Server::handleLogin, this, std::placeholders::_1, std::placeholders::_2);
    }
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
    
        // ایجاد FlightManager
        this->flightManager = make_shared<FlightManager>(&airlines, &costumers, &udpSocket);

        // راه‌اندازی پردازش TCP و UDP
        startServer();
    }
};



#endif // SERVER_HPP