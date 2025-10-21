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

int Server::getAssignedPort(int client_fd) {
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

int Server::HasUniqueUsername(Client_info new_client) {
    for (int i = 0; i < clients.size(); i++) {
        if (strcmp(clients[i]->username, new_client.username) == 0) {
            return -1;
        }
    }
    return 1;
}

int Server::HasValidRole(Client_info new_client) {
    // if (strcmp(new_client.role, ROLE_CODER_STR) == 0) {
    //     return 1;
    // }
    // if (strcmp(new_client.role, ROLE_NAVIGATOR_STR) == 0) {
    //     return 1;
    // }
    // return -1;

    return 1;
}

int Server::checkClientInfo(Client_info new_client) {
    if (HasUniqueUsername(new_client) == -1) {
        send(new_client.client_fd, ERR_USERNAME_STR, strlen(ERR_USERNAME_STR), 0);
        return -1;
    }
    if (HasValidRole(new_client) == -1) {
        send(new_client.client_fd, ERR_ROLE_STR, strlen(ERR_ROLE_STR), 0);
        return -1;
    }
    return 1;
}

int Server::addNewClient(Client_info new_client) {
    if (checkClientInfo(new_client) == -1) {
        return -1;
    }
    Client_info * ptr = new Client_info(new_client);
    
    // Team *team = findTeamByClientName(teams, new_client.username);
    // if (team != nullptr) {
    //     my_print("Client recconected\n");
    //     handleClientReconnection(teams, clients, ptr);
    // } else {
    //     new_client.has_teammate = false;
    // }
    
    clients.push_back(ptr);
    return clients.size();
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
    Client_info new_client;
    if (!receiveClientInfo(new_client_fd, new_client)) {
        my_print("Failed to receive client information.\n");
        closeClientConnection(assigned_ports, new_client_fd, new_port);
        return;
    }
    new_client.port = new_port;
    new_client.client_fd = new_client_fd;

    if (addNewClient(new_client) == -1) {
        send(new_client.client_fd, "ERR: Invalid information", 25, 0);
        closeClientConnection(assigned_ports, new_client.client_fd, new_port);
        return;
    }

    // چاپ اطلاعات کلاینت جدید
    my_print("New client connected: ");
    my_print(new_client.username);
    my_print(" (");
    my_print(new_client.role);
    my_print(") on port ");
    my_print(std::to_string(new_port).c_str());
    my_print("\n");
}

void Server::prepareFdSetForServer(fd_set& read_fds, int& max_fd) {
    FD_ZERO(&read_fds);
    FD_SET(server_fd, &read_fds);
    FD_SET(udp_socket, &read_fds);
    max_fd = std::max(server_fd, udp_socket);
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
        Client_info *client = *it;


        if (FD_ISSET(client->client_fd, &read_fds)) {
            int len = recv(client->client_fd, buffer, sizeof(buffer) - 1, 0);
            if (len > 0) {
                buffer[len] = '\0';
                my_print("[TCP] Client ");
                my_print(client->username);
                my_print(": ");
                my_print(buffer);
                my_print("\n");
            
                // پاسخ به همان کلاینت
                send(client->client_fd, "Received your message", 21, 0);
            
                // Broadcast پیام از طریق UDP
                strcpy(buffer, "Hello to all clients!");
                sendto(udp_socket, buffer, strlen(buffer), 0,
                       (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
            
                ++it;
            } else {
                // handleClientDisconnection(assigned_ports, teams, clients, *it);
            }
        } else {
            ++it;
        }
    }
}

void Server::handleUdpBroadcast(fd_set& read_fds) {
    if (FD_ISSET(udp_socket, &read_fds)) {
        char buffer[1024] = "Hello to all clients!";
        sendto(udp_socket, buffer, strlen(buffer), 0,
               (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
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
                // startGame();
                return;
            }
            // if (strcmp(buffer, "team\n") == 0) {
            //     int count = 0;
            //     my_print("Teams: \n");
            //     for (auto team : teams) {
            //         my_print("Team ");
            //         my_print(to_string(count).c_str());
            //         my_print(": ");
            //         my_print(team->coder->username);
            //         my_print(" - ");
            //         my_print(team->navigator->username);
            //         my_print("\n");
            //         count++;
            //     }
            }
            if (strcmp(buffer, "clients\n") == 0) {
                my_print("Clients: \n");
                for (auto client : clients) {
                    my_print(client->username);
                    my_print(" (");
                    my_print(client->role);
                    my_print(")\n");
                }
            }
        }
    }

void Server::startServer() {
    fd_set read_fds;
    int max_fd;

    // تنظیمات برای ارسال Broadcast
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(udp_port);
    broadcast_addr.sin_addr.s_addr = INADDR_BROADCAST;

    while (true) {
        // handle time
        // if (gameManager->handleTime() == END_GAME) {
        //     break;
        // }

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
        // if (start_flag == -1) {
        //     pairUpClients();
            handleClientMessages(read_fds);
        // } else {
            // handleGameMessages(read_fds);
        // }
        handleUdpBroadcast(read_fds);
    }
}