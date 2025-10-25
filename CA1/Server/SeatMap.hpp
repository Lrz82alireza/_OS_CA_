#ifndef SEATMAP_HPP
#define SEATMAP_HPP

#include <vector>
#include <iostream>
using namespace std;

class SeatMap
{
private:
    int rows;
    int cols;
    vector<vector<bool>> seats;
    int freeSeatsCounter;

    std::pair<int, int> parseSeatLabel(const std::string& seat);

public:
    SeatMap(int r = 0, int c = 0);
    ~SeatMap();

    int setReserves(vector<string> reserves);
    int releaseReserves(vector<string> reserves);
    bool reserve(vector<vector<bool>>& seats_, int row, int col);
    bool reserve(int row, int col);
    bool release(int row, int col);

    // Checker
    bool isPosValid(int row, int col) const;

    // Getters
    int getFreeSeatsCount() const;
    int getRows() const;
    int getCols() const;
    void print() const;
};

#endif
