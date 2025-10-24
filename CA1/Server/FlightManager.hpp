#ifndef FLIGHTMANAGER_HPP
#define FLIGHTMANAGER_HPP

#include "SHARED.hpp"
#include "Client_info.hpp"
#include "SeatMap.hpp"

using namespace std;

struct Flight
{
    string flight_id;
    string origin;
    string destination;
    string time;
    SeatMap seatMap;
};


class FlightManager
{
private:
    
    vector<shared_ptr<User>>* users;

    vector<shared_ptr<Flight>> flights;

    UdpSocket *udpSocket;

    map<int, function<void(shared_ptr<Client_info>, const string&)>> commandHandlers;

    // ___________ DISPATCHER FUNC. _____________
    void handleAddFlight(shared_ptr<Client_info> client, const string& content);
    void handleReserve(shared_ptr<Client_info> client, const string& content);
    void handleConfirm(shared_ptr<Client_info> client, const string& content);
    void handleListFlights(shared_ptr<Client_info> client, const string& content);

    // ___________ checker FUNC. _____________
    bool isFlightIdUnique(const string& flight_id);
    bool checkRolePermission(shared_ptr<Client_info> client, int required_role);

public:
    FlightManager(vector<shared_ptr<User>>* users, UdpSocket *udpSocket) 
        :users(users), udpSocket(udpSocket) {
        commandHandlers[ADD_FLIGHT_N] = std::bind(&FlightManager::handleAddFlight, this, std::placeholders::_1, std::placeholders::_2);
        commandHandlers[RESERVE_N] = std::bind(&FlightManager::handleReserve, this, std::placeholders::_1, std::placeholders::_2);
        commandHandlers[CONFIRM_N] = std::bind(&FlightManager::handleConfirm, this, std::placeholders::_1, std::placeholders::_2);
        commandHandlers[LIST_FLIGHTS_N] = std::bind(&FlightManager::handleListFlights, this, std::placeholders::_1, std::placeholders::_2);
            
    }
    ~FlightManager() {}

    void handleMessage(shared_ptr<Client_info> client_info, const char* buffer, int len);
};


#endif