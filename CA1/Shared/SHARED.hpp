// SHARED.h

#ifndef SHARED_HPP
#define SHARED_HPP

#include "Airline.hpp"
#include "Costumer.hpp"

#include <map>
#include <functional>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <thread>
#include <chrono>
#include <set>

#define PASS "PASS"
#define FAIL "FAIL"

#define REGISTER_STR "REGISTER"
#define LOGIN_STR "LOGIN"
#define ADD_FLIGHT_STR "ADD_FLIGHT"
#define RESERVE_STR "RESERVE"
#define CONFIRM_STR "CONFIRM"
#define LIST_FLIGHTS_STR "LIST_FLIGHTS"

#define REGISTER_N 1
#define LOGIN_N 2
#define ADD_FLIGHT_N 3
#define RESERVE_N 4
#define CONFIRM_N 5
#define LIST_FLIGHTS_N 6

struct UdpChannel {
    int fd;
    int port;
    sockaddr_in addr;
};

struct UdpSocket {
    UdpChannel airLine;
    UdpChannel customer;
};

struct State
{
    bool submitted[3] = {false, false, false};
    int time_submitted[3] = {0, 0, 0};
};

struct Submission
{
    char team_name[50] = "";
    char problem_id[20] = "";
    char code[2000] = "";
};

struct Team
{
    Client_info* coder = nullptr;
    Client_info* navigator = nullptr;
    float score[3] = {0, 0, 0};
    State state;
    Submission submission;
};

struct Message
{
    int type;
    std::string content;
};


void my_print(const char* str);
std::string to_string(int value);
std::string to_string(double value);
std::string to_string(const char* value);

int create_socket(bool is_udp, bool is_broadcast);
void bind_socket(int sock_fd, int port, bool is_udp);
void connect_socket(int sock_fd, const char* ip, int port);
sockaddr_in makeBroadcastAddress(int port, const std::string& ip = "255.255.255.255");


std::string extractType(const std::string& input);
void sendMsgToTeam(Team* team, const std::string& msg);

int createEvaluationSocket(const char* server_ip);

void read_line(std::string& input);
Message decodeMessage(const std::string& message);

Team* findTeamByClientName(const std::vector<Team*>& teams, const std::string& clientName);
Client_info *findPartnerInTeam(Team *team, const std::string& clientName);

void handleClientDisconnection(std::set<int> assigned_ports, std::vector<Team *> &teams, std::vector<Client_info *> &clients, Client_info *client);
void closeClientConnection(std::set<int> assigned_ports, int client_fd, int port);

Client_info * handleClientReconnection(std::vector<Team *> &teams, std::vector<Client_info *> &clients, Client_info *client);
#endif // SHARED_HPP
