#include "Client_info.hpp"

std::string Client_info::getUserName()
{
    if (this->user != nullptr) {
        return this->user->username;
    }
    return "";
}
