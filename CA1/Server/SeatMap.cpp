#include "SeatMap.hpp"

SeatMap::SeatMap(int r, int c) : rows(r), cols(c)
{
    seats.assign(rows, vector<bool>(cols, false));
    freeSeatsCounter = rows * cols;
}

SeatMap::~SeatMap() {}

bool SeatMap::isPosValid(int row, int col) const
{
    return (row >= 0 && row < rows && col >= 0 && col < cols);
}

bool SeatMap::reserve(int row, int col)
{
    if (!isPosValid(row, col) || seats[row][col]) return false;
    seats[row][col] = true;
    freeSeatsCounter--;
    return true;
}

bool SeatMap::release(int row, int col)
{
    if (!isPosValid(row, col) || !seats[row][col]) return false;
    seats[row][col] = false;
    freeSeatsCounter++;
    return true;
}

int SeatMap::getFreeSeatsCount() const
{
    return freeSeatsCounter;
}

int SeatMap::getRows() const
{
    return rows;
}

int SeatMap::getCols() const
{
    return cols;
}

void SeatMap::print() const
{
    cout << "Seat Map (" << rows << "x" << cols << "):\n";
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
            cout << (seats[i][j] ? "[X]" : "[ ]");
        cout << endl;
    }
    cout << "Free Seats: " << freeSeatsCounter << endl;
}
