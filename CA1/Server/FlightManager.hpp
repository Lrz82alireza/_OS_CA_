#ifndef FLIGHTMANAGER_HPP
#define FLIGHTMANAGER_HPP

#include "SHARED.hpp"

using namespace std;

class FlightManager
{
private:
    
    vector<Airline*>* airlines;
    vector<Costumer*>* costumers;

    UdpSocket *udpSocket;

    map<int, function<void(Client_info*, const string&)>> commandHandlers;

    // ___________ DISPATCHER FUNC. _____________
    void handleAddFlight(Client_info* client, const string& content);
    void handleReserveFlight(Client_info* client, const string& content);
    void handleConfirmReservation(Client_info* client, const string& content);

public:
    FlightManager(vector<Airline*>* airlines, vector<Costumer*>* costumers, UdpSocket *udpSocket) 
        :airlines(airlines), costumers(costumers), udpSocket(udpSocket) {
            
    }
    ~FlightManager() {}
};


#endif