// Server.cpp
#include "Server.hpp"

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
    return -1;
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