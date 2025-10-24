#include "FlightManager.hpp"

void FlightManager::handleAddFlight(shared_ptr<Client_info> client, const string &content)
{
    checkRolePermission(client, ROLE_AIRLINE);
    vector<string> ss = split(content, ' ');
    if (ss.size() != 7) {
        send(client->client_fd, "ERR: Invalid ADD_FLIGHT format", strlen("ERR: Invalid ADD_FLIGHT format"), 0);
        return;
    }
}

bool FlightManager::checkRolePermission(shared_ptr<Client_info> client, int required_role)
{
    if (client->user == nullptr || client->user->role != required_role) {
        send(client->client_fd, "ERR: Permission denied", strlen("ERR: Permission denied"), 0);
        return false;
    }
    return true;
}

void FlightManager::handleMessage(shared_ptr<Client_info> client_info, const char *buffer, int len)
{
    Message msg = decodeMessage(std::string(buffer, len));
    if (msg.type == -1) {
        my_print("Invalid message from client.\n");
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