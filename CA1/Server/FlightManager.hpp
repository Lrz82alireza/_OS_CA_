#ifndef FLIGHTMANAGER_HPP
#define FLIGHTMANAGER_HPP

#include "SHARED.hpp"
#include "Client_info.hpp"
#include "SeatMap.hpp"

#define ERR_FLIGHT_ID_STR "ERROR FlightIDAlreadyExists"
#define ERR_CONFIRM_STR "ERROR ReservationExpired"
#define APPROVED_ADD_FLIGHT_STR "FLIGHT_ADDED OK"
#define APPROVED_CONFIRM_STR "CONFIRMATION OK"
#define APPROVED_CANCEL_STR "CANCELLED OK"

#define TEMPORARY "TEMPORARY"
#define CONFIRMED "CONFIRMED"
#define EXPIRED "EXPIRED"

#define EXPIRE_TIME 30

using namespace std;

struct Reservation {
    static int count_id;
    int reservation_id;
    std::string flight_id;
    std::string username;
    std::vector<std::string> seats;
    std::string status;                 // TEMPORARY / CONFIRMED / EXPIRED
    time_t timestamp;

    Reservation(const std::string& flight,
                const std::string& user,
                const std::vector<std::string>& seats_)
        : flight_id(flight),
        username(user),
        seats(seats_),  
        status(TEMPORARY),
        timestamp(time(nullptr))
    {
        reservation_id = count_id++;
    }
};



class FlightManager
{
private:
    
    vector<shared_ptr<User>>* users;
    vector<shared_ptr<Flight>> flights;
    vector<shared_ptr<Reservation>> reservations;

    UdpSocket *udpSocket;

    map<int, function<void(shared_ptr<Client_info>, const string&)>> commandHandlers;

    // ___________ DISPATCHER FUNC. _____________
    void handleAddFlight(shared_ptr<Client_info> client, const string& content);
    void handleReserve(shared_ptr<Client_info> client, const string& content);
    void handleConfirm(shared_ptr<Client_info> client, const string& content);
    void handleCancel(shared_ptr<Client_info> client, const string& content);
    void handleListFlights(shared_ptr<Client_info> client, const string& content);

    // ___________ checker FUNC. _____________
    bool isFlightIdUnique(const string& flight_id);
    bool checkRolePermission(shared_ptr<Client_info> client, int required_role);

    // ___________ H-Func. _____________
    shared_ptr<Reservation> findReservationById(int reservation_id);
public:
    FlightManager(vector<shared_ptr<User>>* users, UdpSocket *udpSocket) 
        :users(users), udpSocket(udpSocket) {
        commandHandlers[ADD_FLIGHT_N] = std::bind(&FlightManager::handleAddFlight, this, std::placeholders::_1, std::placeholders::_2);
        commandHandlers[RESERVE_N] = std::bind(&FlightManager::handleReserve, this, std::placeholders::_1, std::placeholders::_2);
        commandHandlers[CONFIRM_N] = std::bind(&FlightManager::handleConfirm, this, std::placeholders::_1, std::placeholders::_2);
        commandHandlers[LIST_FLIGHTS_N] = std::bind(&FlightManager::handleListFlights, this, std::placeholders::_1, std::placeholders::_2);
        commandHandlers[CANCEL_N] = std::bind(&FlightManager::handleCancel, this, std::placeholders::_1, std::placeholders::_2);
    }
    ~FlightManager() {}

    void handleMessage(shared_ptr<Client_info> client_info, const char* buffer, int len);
    void expireTemporaryReservations();
};


#endif