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

public:
    SeatMap(int r = 0, int c = 0);
    ~SeatMap();

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
