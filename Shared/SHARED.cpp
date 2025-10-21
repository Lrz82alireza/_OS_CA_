// SHARED.cpp
#include "SHARED.h"
#include <unistd.h>
#include <sstream>
#include <string.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>


// پیاده‌سازی تابع print
void my_print(const char* str) {
    write(STDOUT_FILENO, str, strlen(str));
}

// تبدیل عدد صحیح به رشته
std::string to_string(int value) {
    char buffer[12]; // فضای کافی برای نمایش عدد صحیح (حداکثر 11 رقم + null)
    sprintf(buffer, "%d", value);
    return std::string(buffer);
}

// تبدیل عدد اعشاری به رشته
std::string to_string(double value) {
    char buffer[32]; // فضای کافی برای نمایش عدد اعشاری
    sprintf(buffer, "%.6f", value); // نمایش تا 6 رقم اعشار
    return std::string(buffer);
}

// تبدیل const char* به رشته
std::string to_string(const char* value) {
    if (value == nullptr) {
        return std::string(""); // جلوگیری از کرش در صورت nullptr
    }
    return std::string(value);
}

// تابع برای ایجاد سوکت
int create_socket(bool is_udp, bool is_broadcast) {
    int sock_fd = socket(AF_INET, (is_udp ? SOCK_DGRAM : SOCK_STREAM), 0);
    if (sock_fd < 0) {
        my_print("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // فعال‌سازی SO_REUSEADDR برای امکان استفاده مجدد از پورت
    int reuseAddr = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) < 0) {
        perror("Failed to set SO_REUSEADDR");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &reuseAddr, sizeof(reuseAddr)) < 0) {
        perror("Failed to set SO_REUSEPORT");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // فعال‌سازی Broadcast در صورت نیاز
    if (is_udp && is_broadcast) {
        int broadcastEnable = 1;
        if (setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
            perror("Failed to enable broadcast");
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
    }

    return sock_fd;
}


// تابع برای Bind کردن سوکت به یک پورت
void bind_socket(int sock_fd, int port, bool is_broadcast) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = is_broadcast ? INADDR_BROADCAST : INADDR_ANY;

    if (bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
}

// تابع برای اتصال کلاینت به سرور (TCP)
void connect_socket(int sock_fd, const char* server_ip, int port) {
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        my_print("Invalid address");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        my_print("Connecting failed \n");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
}

std::string extractType(const std::string& input) {
    if (input.empty() || input[0] != '/') return "";

    size_t spacePos = input.find(' ');
    if (spacePos == std::string::npos) {
        return input.substr(1);  // اگر فاصله‌ای نبود، کل رشته بعد از `/` رو برگردون
    }
    
    return input.substr(0, spacePos);  // نوع پیام (type) رو استخراج کن
}

void sendMsgToTeam(Team* team, const std::string& msg) {
    if (team->coder != nullptr) {
        send(team->coder->client_fd, msg.c_str(), msg.length(), 0);
    }
    if (team->navigator != nullptr) {
        send(team->navigator->client_fd, msg.c_str(), msg.length(), 0);
    }
}

int createEvaluationSocket(const char* server_ip) {
    // ایجاد سوکت TCP
    int sock_fd = create_socket(false, false);

    // اتصال به سرور ارزیابی
    connect_socket(sock_fd, server_ip, 65432);

    return sock_fd;
}

void read_line(std::string& input) {
    input.clear();
    char buffer[256];
    ssize_t bytesRead = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);

    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        input = buffer;

        if (!input.empty() && input.back() == '\n') {
            input.pop_back();
        }
    }
}

Message decodeMessage(const std::string& message) {
    std::string type = extractType(message);
    Message msg = {-1, ""};

    if (type == CODE_STR) {
        msg.type = CODE_N;
    } else if (type == CHAT_STR) {
        msg.type = CHAT_N;
    } else if (type == SUBMIT_STR) {
        msg.type = SUBMIT_N;
    } else if (type == SAVE_STR) {
        msg.type = SAVE_N;
    } else {
        std::string tmp = "Invalid message type: ";
        tmp += type;
        tmp += "\n";
        my_print(tmp.c_str());
    }
    msg.content = message.substr(type.length() + 1);

    return msg;
}

std::string codeSpace() {
    std::string tmp = "Start writing your code. Type 'save' on a new line to finish.\n";  
    my_print(tmp.c_str());
    std::string current_code = "/code ";

    std::string line;
    while (true) {
        read_line(line);
        if (line == SAVE_STR) {
            my_print("Code saved successfully.\n");
            break;
        }
        current_code += line;
    }
    my_print(current_code.c_str());
    return current_code;
}

Team* findTeamByClientName(const std::vector<Team*>& teams, const std::string& clientName) {
    for (Team* team : teams) {
        if (team->coder != nullptr && team->coder->username == clientName) {
            return team;
        }
        if (team->navigator != nullptr && team->navigator->username == clientName) {
            return team;
        }
    }
    return nullptr;
}

Client_info *findPartnerInTeam(Team *team, const std::string& clientName)
{
    if (team != nullptr) {
        if (strcmp(team->coder->username, clientName.c_str()) == 0) {
            return team->navigator;
        } else {
            return team->coder;
        }
    } 

    return nullptr;
}

void handleClientDisconnection(std::set<int> assigned_ports, std::vector<Team *> &teams, std::vector<Client_info *> &clients, Client_info *client)
{
    my_print("Client disconnected.\n");
    closeClientConnection(assigned_ports, client->client_fd, client->port);

    Team *team = findTeamByClientName(teams, client->username);
    Client_info *partner = findPartnerInTeam(team, client->username);
    if (partner != nullptr) {
        std::string msg = "Your teammate disconnected.\n";
        send(partner->client_fd, msg.c_str(), strlen(msg.c_str()), 0);
    }

    // erase from clients
    auto it = std::find(clients.begin(), clients.end(), client);
    if (it != clients.end()) {
        clients.erase(it);
    }
}

// تابع برای بستن اتصال اولیه با کلاینت
void closeClientConnection(std::set<int> assigned_ports, int client_fd, int port) {
    assigned_ports.erase(port);
    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
}

Client_info * handleClientReconnection(std::vector<Team *> &teams, std::vector<Client_info *> &clients, Client_info *client)
{
    Team *team = findTeamByClientName(teams, client->username);
    Client_info *partner = findPartnerInTeam(team, client->username);
    if (partner != nullptr) {
        std::string msg = "Your teammate reconnected.\n";
        send(partner->client_fd, msg.c_str(), strlen(msg.c_str()), 0);
    }

    client->has_teammate = true;
    if (strcmp(team->coder->username, client->username) == 0) {
        team->coder = client;
    } else {
        team->navigator = client;
    }

    return client;
}
