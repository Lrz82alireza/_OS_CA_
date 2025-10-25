#include "SeatMap.hpp"

std::pair<int, int> SeatMap::parseSeatLabel(const std::string& seat) {
    if (seat.size() < 2) return {-1, -1};
    if (!std::isalpha(seat[0])) return {-1, -1};

    char rowChar = std::toupper(seat[0]);
    int row = rowChar - 'A';
    
    std::string colStr = seat.substr(1);
    for (char c : colStr)
        if (!std::isdigit(c)) return {-1, -1};

    int col;
    try {
        col = std::stoi(colStr) - 1;
    } catch (...) {
        return {-1, -1};
    }

    if (row < 0 || col < 0) return {-1, -1};
    return {row, col};
}

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

// returns -1 = InvalidInput, -2 = has been reserevedBefore
int SeatMap::setReserves(std::vector<std::string> reserves) {
    std::vector<std::vector<bool>> final_res = this->seats;

    for (auto& res : reserves) {
        auto pos = parseSeatLabel(res);
        if (pos.first == -1 || pos.second == -1)
            return -1;
        if (!isPosValid(pos.first, pos.second))
            return -1;
        if (!reserve(final_res, pos.first, pos.second))
            return -2;
    }

    this->seats = final_res;
    return 1;
}

int SeatMap::releaseReserves(vector<string> reserves)
{
    for (auto& res : reserves) {
        auto pos = parseSeatLabel(res);
        if (pos.first == -1 || pos.second == -1)
            return -1;
        if (!isPosValid(pos.first, pos.second))
            return -1;
        if (!release(pos.first, pos.second))
            return -1;
    }
    return 0;
}

// return false if seat was reserved
bool SeatMap::reserve(vector<vector<bool>>& seats_, int row, int col)
{
    if (seats_[row][col]) {
        return false;
    }
    seats_[row][col] = true;
    freeSeatsCounter--;
    return true;
}

bool SeatMap::reserve(int row, int col)
{
    return reserve(this->seats, row, col);
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
