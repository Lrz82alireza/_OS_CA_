#ifndef FLIGHTMANAGER_HPP
#define FLIGHTMANAGER_HPP

#include "SHARED.hpp"
#include "Client_info.hpp"

using namespace std;

class FlightManager
{
private:
    
    vector<shared_ptr<Airline>>* airlines;
    vector<shared_ptr<Customer>>* customers;

    UdpSocket *udpSocket;

    map<int, function<void(shared_ptr<Client_info>, const string&)>> commandHandlers;

    // ___________ DISPATCHER FUNC. _____________
    void handleAddFlight(shared_ptr<Client_info> client, const string& content);
    void handleReserve(shared_ptr<Client_info> client, const string& content);
    void handleConfirm(shared_ptr<Client_info> client, const string& content);
    void handleLastFlights(shared_ptr<Client_info> client, const string& content);

public:
    FlightManager(vector<shared_ptr<Airline>>* airlines, vector<shared_ptr<Customer>>* customers, UdpSocket *udpSocket) 
        :airlines(airlines), customers(customers), udpSocket(udpSocket) {
            
    }
    ~FlightManager() {}
};


#endif