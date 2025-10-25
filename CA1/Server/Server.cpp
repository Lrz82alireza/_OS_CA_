// Server.cpp
#include "Server.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        my_print("Usage: ");
        my_print(argv[0]);
        my_print(" <port>\n");
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    Server server(port);
    server.start();

    return 0;
}

void Server::handleRegister(shared_ptr<Client_info> client, const string &content)
{
    vector<std::string> ss = split(content, ' ');
    if (ss.size() != 4) {
        send(client->client_fd, "ERR: Invalid REGISTER format", strlen("ERR: Invalid REGISTER format"), 0);
        return;
    }
    string role = ss[1];
    string username = ss[2];
    string password = ss[3];
    
    
    if (registerClient(client, username, password, role) == -1) {
        return;
    }
    send(client->client_fd, APPROVED_REGISTER_STR, strlen(APPROVED_REGISTER_STR), 0);
    my_print("Client ");
    my_print(username.c_str());
    my_print(" with role ");
    my_print(role.c_str());
    my_print(" registered successfully.\n");

    sendBroadcastMessage(this->udpSocket.airLine.fd, this->udpSocket.airLine.addr, "BROADCAST NEW_USER " + username + " " + role + " \n");
    // sendBroadcastMessage(this->udpSocket.customer.fd, this->udpSocket.customer.addr, "BROADCAST NEW_USER " + username + " " + role + " \n");
}

void Server::handleLogin(shared_ptr<Client_info> client, const string &content)
{
    vector<std::string> ss = split(content, ' ');
    if (ss.size() != 3) {
        send(client->client_fd, "ERR: Invalid LOGIN format", strlen("ERR: Invalid LOGIN format"), 0);
        return;
    }

    string username = ss[1];
    string password = ss[2];

    shared_ptr<User> user = findUser(username, password);
    if (user == nullptr) {
        send(client->client_fd, ERR_LOGIN_STR, strlen(ERR_LOGIN_STR), 0);
        return;
    }
    client->user = user;

    sendUdpPort(client);
    send(client->client_fd, APPROVED_LOGIN_STR, strlen(APPROVED_LOGIN_STR), 0); 
}

int Server::getAssignedPort(int client_fd)
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    getsockname(client_fd, (struct sockaddr*)&addr, &addr_len);
    return ntohs(addr.sin_port);
}

int Server::acceptNewClient(int server_fd) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("accept() failed");
        return -1;
    }
    return client_fd;
}

int Server::assignNewPort() {
    // بررسی پورت‌های قبلی و اختصاص مجدد در صورت امکان
    for (int port = BASE_PORT; port < BASE_PORT + clients.size(); ++port) {
        if (assigned_ports.find(port) == assigned_ports.end()) {  // اگر پورت قبلاً اختصاص داده نشده است
            assigned_ports.insert(port);
            return port;
        }
    }

    // اگر هیچ پورتی از قبل آزاد نبود، یک پورت جدید اختصاص بده
    int new_port = BASE_PORT + clients.size();
    assigned_ports.insert(new_port);
    return new_port;
}

void Server::sendNewPortToClient(int client_fd, int new_port) {
    char port_msg[10];
    sprintf(port_msg, "%d", new_port);
    send(client_fd, port_msg, strlen(port_msg), 0);
}

int Server::waitForClientOnNewPort(int new_server_fd) {
    struct sockaddr_in new_client_addr;
    socklen_t new_client_len = sizeof(new_client_addr);
    int new_client_fd = accept(new_server_fd, (struct sockaddr*)&new_client_addr, &new_client_len);
    if (new_client_fd < 0) {
        perror("accept() failed on new port");
        return -1;
    }
    return new_client_fd;
}

bool Server::receiveClientInfo(int client_fd, Client_info &new_client) {
    int len = recv(client_fd, &new_client, sizeof(new_client), 0);
    if (len <= 0) {
        return false;
    }
    return true;
}

// _____________ Check Client Info _____________

int Server::HasUniqueUsername(string username) {
    for (int i = 0; i < users.size(); i++) {
        if (users[i]->username == username) {
            return -1;
        }
    }
    return 1;
}

int Server::HasValidRole(string role) {
    if (strcmp(role.c_str(), ROLE_AIRLINE_STR) == 0) {
        return ROLE_AIRLINE;
    }
    if (strcmp(role.c_str(), ROLE_CUSTOMER_STR) == 0) {
        return ROLE_CUSTOMER;
    }
    return -1;
}

int Server::checkUserInfo(Client_info new_client, string username, string role) {
    if (HasUniqueUsername(username) == -1) {
        send(new_client.client_fd, ERR_USERNAME_STR, strlen(ERR_USERNAME_STR), 0);
        return -1;
    }
    int res;
    if ((res = HasValidRole(role)) == -1) {
        send(new_client.client_fd, ERR_ROLE_STR, strlen(ERR_ROLE_STR), 0);
        return -1;
    }
    return res;
}

int Server::registerClient(shared_ptr<Client_info> client, string username, string password, string role_str) {
    int role;
    if ((role = checkUserInfo(*client, username, role_str)) == -1) {
        return -1;
    }

    shared_ptr<User> user = std::make_shared<User>(username, password, role);
    if (role == ROLE_AIRLINE) {
        user = std::make_shared<Airline>(*user);
        role = ROLE_AIRLINE;
    } else if (role == ROLE_CUSTOMER) {
        user = std::make_shared<Customer>(*user);
        role = ROLE_CUSTOMER;
    }   

    users.push_back(user);    

    return 1;
}

shared_ptr<User> Server::findUser(string username, string password)
{
    shared_ptr<User> found_user = nullptr;
    for (const auto& user : users) {
        if (user->username == username && user->password == password) {
            found_user = user;
            break;
        }
    }

    return found_user;
}

void Server::sendUdpPort(shared_ptr<Client_info> client)
{
    std::string msg = "PORT: ";
    if (client->user->role == ROLE_AIRLINE) {
        msg += std::to_string(this->udpSocket.airLine.port) + " ";
    } else if (client->user->role == ROLE_CUSTOMER) {
        msg += std::to_string(this->udpSocket.customer.port) + " ";
    }
    
    
    size_t totalSent = 0;
    const char* data = msg.c_str();
    size_t msgSize = msg.size();

    while (totalSent < msgSize) {
        ssize_t sent = send(client->client_fd, data + totalSent, msgSize - totalSent, 0);
        if (sent <= 0) {
            perror("send failed");
            return;
        }
        totalSent += sent;
    }
}

void Server::installAlarm() {
    struct sigaction sa{};
    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; 

    sigaction(SIGALRM, &sa, nullptr);
    alarm(1);
}

void Server::handleNewClient(int server_fd) {
    // پذیرش اتصال اولیه
    int client_fd = acceptNewClient(server_fd);
    if (client_fd < 0) return;

    // اختصاص پورت جدید به کلاینت
    int new_port = assignNewPort();

    // ایجاد سوکت جدید و bind کردن آن به پورت جدید
    int new_server_fd = create_socket(false, false);
    bind_socket(new_server_fd, new_port, false);

    if (listen(new_server_fd, 5) < 0) {  // بررسی خطا در listen
        perror("Failed to listen on new port");
        close(new_server_fd);
        close(client_fd);
        return;
    }

    // ارسال پورت جدید به کلاینت
    sendNewPortToClient(client_fd, new_port);

    // بستن اتصال اولیه
    close(client_fd);

    // انتظار برای اتصال مجدد کلاینت به پورت جدید
    my_print("Waiting for client on new port...\n");
    int new_client_fd = waitForClientOnNewPort(new_server_fd);
    
    // بستن سوکت سرور جدید بعد از قبول اتصال
    close(new_server_fd);

    if (new_client_fd < 0) return;

    // دریافت اطلاعات از کلاینت
    // if (!receiveClientInfo(new_client_fd, new_client)) {
        //     my_print("Failed to receive client information.\n");
        //     closeClientConnection(assigned_ports, new_client_fd, new_port);
        //     return;
        // }
        
    std::shared_ptr<Client_info> new_client = std::make_shared<Client_info>();
    new_client->port = new_port;
    new_client->client_fd = new_client_fd;

    clients.push_back(new_client);
        // if (addNewClient(new_client) == -1) {
    //     send(new_client.client_fd, "ERR: Invalid information", 25, 0);
    //     closeClientConnection(assigned_ports, new_client.client_fd, new_port);
    //     return;
    // }

    // چاپ اطلاعات کلاینت جدید
    my_print("New client connected ");
    my_print("on port ");
    my_print(std::to_string(new_port).c_str());
    my_print("\n");
}

void Server::prepareFdSetForServer(fd_set& read_fds, int& max_fd) {
    FD_ZERO(&read_fds);
    FD_SET(this->server_fd, &read_fds);
    FD_SET(this->udpSocket.airLine.fd, &read_fds);
    FD_SET(this->udpSocket.customer.fd, &read_fds);
    max_fd = std::max(max(server_fd, udpSocket.customer.fd), udpSocket.airLine.fd);
    for (const auto& client : clients) {
        FD_SET(client->client_fd, &read_fds);
        if (client->client_fd > max_fd) max_fd = client->client_fd;
    }
}

void Server::handleNewConnections(fd_set& read_fds) {
    if (FD_ISSET(server_fd, &read_fds)) {
        handleNewClient(server_fd);
    }
}

void Server::handleClientMessages(fd_set& read_fds) {
    char buffer[1024];
    for (auto it = clients.begin(); it != clients.end(); ) {
        std::shared_ptr<Client_info> client = *it;


        if (FD_ISSET(client->client_fd, &read_fds)) {
            int len = recv(client->client_fd, buffer, sizeof(buffer) - 1, 0);
            if (len > 0) {
                if (this->start_flag) {
                    if (client->user != nullptr) {
                        handleLoggedInMessages(client, buffer, len);
                    } else {
                        handleLoggedOutMessages(client, buffer, len);
                    }
                } else {                    
                    buffer[len] = '\0';
                    my_print("[TCP] Client port: ");
                    my_print(client->port);
                    my_print(", fd: ");
                    my_print(client->client_fd);
                    my_print(", sent: ");
                    my_print(buffer);
                    my_print("\n");
                
                    // پاسخ به همان کلاینت
                    send(client->client_fd, "Received your message", 21, 0);
                
                }

                ++it;
            } else {
                // client disconnection
                // handleClientDisconnection(assigned_ports, teams, clients, *it);
            }
        } else {
            ++it;
        }
    }
}

void Server::handleLoggedOutMessages(shared_ptr<Client_info> client_info, const char* buffer, int len) {
    Message msg = decodeMessage(std::string(buffer, len));
    if (msg.type == -1) {
        my_print("Invalid message from logged-out client.\n");
        send(client_info->client_fd, "ERR: Invalid message type", 25, 0);
        return;
    }

    auto it = commandHandlers.find(msg.type);
    if (it != commandHandlers.end()) {
        it->second(client_info, msg.content);
    } else {
        // my_print("Unknown command type.\n");
        send(client_info->client_fd, "ERR: Invalid message type", 21, 0);
    }
}

void Server::handleLoggedInMessages(shared_ptr<Client_info> client_info, const char *buffer, int len)
{
    if (this->flightManager != nullptr) {
        this->flightManager->handleMessage(client_info, buffer, len);
    } else {
        my_print("FlightManager not initialized.\n");
        send(client_info->client_fd, "ERR: Server not ready", 21, 0);
    }
}


void Server::handleKeyboardInput(fd_set& read_fds) {
    char buffer[1024];
    int len = 0;

    // بررسی ورودی از کیبورد
    if (FD_ISSET(STDIN_FILENO, &read_fds)) {
        len = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
        if (len > 0) {
            buffer[len] = '\0';

            // پردازش ورودی
            if (strcmp(buffer, "start\n") == 0) {
                if (this->start_flag) {
                    my_print("Server already started you dumbass!!\n");
                    return;
                } else {
                    this->start_flag = true;
                    my_print("Server started!\n");
                }
                return;
            }
            if (strcmp(buffer, "clients\n") == 0) {
                my_print("Clients: \n");
                for (auto client : clients) {
                    my_print(client->port);
                    my_print(" (");
                    my_print(client->client_fd);
                    my_print(")\n");
                }
            }
            if (strcmp(buffer, "users\n") == 0) {
                my_print("Users: \n");
                for (auto user : users) {
                    my_print(user->username + " " + user->password + " " + to_string(user->role));
                    my_print("\n");
                }
            }
            if (strcmp(buffer, "udp\n") == 0) {
                sendBroadcastMessage(this->udpSocket.airLine.fd, this->udpSocket.airLine.addr, "BROADCAST: AIRLINE");
                sendBroadcastMessage(this->udpSocket.customer.fd, this->udpSocket.customer.addr, "BROADCAST: CUSTOMER");
            }
        }
    }
}

void Server::startServer() {
    fd_set read_fds;
    int max_fd;

    // تنظیمات برای ارسال Broadcast
    udpSocket.airLine.addr = makeBroadcastAddress(udpSocket.airLine.port);
    udpSocket.customer.addr = makeBroadcastAddress(udpSocket.customer.port);

    while (true) {
        prepareFdSetForServer(read_fds, max_fd);

        // اضافه کردن STDIN_FILENO به مجموعه فایل‌ها
        FD_SET(STDIN_FILENO, &read_fds);
        max_fd = std::max(max_fd, STDIN_FILENO); 

        // تنظیم timeout برای select
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000000; // 0.5 ثانیه

        int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        if (activity < 0) {
            my_print("select() failed");
            break;
        }
    
        // I/O Processing
        handleKeyboardInput(read_fds);
    
        handleNewConnections(read_fds);
        // if (!start_flag) {
            handleClientMessages(read_fds);
        // }

        if (g_tick) {
            g_tick = 0;
            flightManager->expireTemporaryReservations();
            alarm(1);
        }
    }
}