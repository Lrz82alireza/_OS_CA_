#include "FlightManager.hpp"

void FlightManager::handleAddFlight(shared_ptr<Client_info> client, const string &content)
{
    checkRolePermission(client, ROLE_AIRLINE);
    vector<string> ss = split(content, ' ');
    if (ss.size() != 7) {
        send(client->client_fd, "ERR: Invalid ADD_FLIGHT format", strlen("ERR: Invalid ADD_FLIGHT format"), 0);
        return;
    }

    Flight new_flight;
    new_flight.flight_id = ss[1];
    new_flight.origin = ss[2];
    new_flight.destination = ss[3];
    new_flight.time = ss[4];
    int rows = stoi(ss[5]);
    int cols = stoi(ss[6]);

    if (!isFlightIdUnique(new_flight.flight_id)) {
        send(client->client_fd, ERR_FLIGHT_ID_STR, strlen(ERR_FLIGHT_ID_STR), 0);
        return;
    }
    flights.push_back(make_shared<Flight>(Flight{new_flight.flight_id, new_flight.origin, new_flight.destination, new_flight.time, SeatMap(rows, cols)}));
    send(client->client_fd, APPROVED_ADD_FLIGHT_STR, strlen(APPROVED_ADD_FLIGHT_STR), 0);

    string broadcast_msg = "BROADCAST NEW_FLIGHT " + new_flight.flight_id + " " + new_flight.origin + " " + new_flight.destination + " " + new_flight.time + "\n";
    sendBroadcastMessage(udpSocket->customer.fd, udpSocket->customer.addr, broadcast_msg);
}

bool FlightManager::isFlightIdUnique(const string &flight_id)
{
    for (const auto& flight : flights) {
        if (flight->flight_id == flight_id) {
            return false;
        }
    }
    return true;
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