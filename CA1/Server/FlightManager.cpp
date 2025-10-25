#include "FlightManager.hpp"

void FlightManager::handleAddFlight(shared_ptr<Client_info> client, const string &content)
{
    if (!checkRolePermission(client, ROLE_AIRLINE)) {
        return;
    }
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

void FlightManager::handleReserve(shared_ptr<Client_info> client, const string &content)
{
    if (!checkRolePermission(client, ROLE_CUSTOMER)) {
        return;
    }
    vector<string> ss = split(content, ' ');
    if (ss.size() >= 3) {
        send(client->client_fd, "ERR: Invalid RESERVE format", strlen("ERR: Invalid RESERVE format"), 0);
        return;
    }

    Reservation res(ss[1], client->getUserName(), std::vector<std::string>(ss.begin() + 2, ss.end()));

    shared_ptr<Flight> flight = findFlightById(flights, ss[1]);
    if (flight == nullptr) {
        send(client->client_fd, "ERR: Invalid Flight ID", strlen("ERR: Invalid Flight ID"), 0);
        return;
    }

    int temp_res = flight->seatMap.setReserves(res.seats);
    if (temp_res == -1) {
        send(client->client_fd, "ERR: Invalid Seat Label", strlen("ERR: Invalid Seat Label"), 0);
        return;
    } else if (temp_res == -2) {
        send(client->client_fd, "ERR: Seat Already Reserved", strlen("ERR: Seat Already Reserved"), 0);
        return;
    }

    reservations.push_back(make_shared<Reservation>(res));
    string msg = "RESERVED TEMP " + to_string(res.reservation_id) + " EXPIRE_IN " + to_string(EXPIRE_TIME) + "\n";
    send(client->client_fd, msg.c_str(), msg.length(), 0);
    
}

void FlightManager::handleListFlights(shared_ptr<Client_info> client, const string &content)
{
    string msg = "";
    for (const auto& flight : flights) {
        int max_seats = flight->seatMap.getRows() * flight->seatMap.getCols();
        msg += "FLIGHT " + flight->flight_id + " " + flight->origin + " " + flight->destination + " " 
            + flight->time + " SEATS_AVAILABLE " + to_string(flight->seatMap.getFreeSeatsCount()) + "/" 
            + to_string(max_seats) + "\n";
    }
    if (msg.empty()) {
        msg = "NO_FLIGHTS\n";
    }
    send(client->client_fd, msg.c_str(), msg.length(), 0);
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

void FlightManager::expireTemporaryReservations()
{
    time_t current_time = time(nullptr);
    for (auto it = reservations.begin(); it != reservations.end(); ) {
        shared_ptr<Reservation> res = *it;
        if (res->status == TEMPORARY) {
            double elapsed = difftime(current_time, res->timestamp);
            if (elapsed >= EXPIRE_TIME) {
                // Release seats
                shared_ptr<Flight> flight = findFlightById(flights, res->flight_id);
                if (flight != nullptr) {
                    flight->seatMap.releaseReserves(res->seats);
                }
                res->status = EXPIRED;
            }
        } 
        ++it;
    }
}
