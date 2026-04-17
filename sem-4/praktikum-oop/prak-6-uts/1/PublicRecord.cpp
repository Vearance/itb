#include "PublicRecord.hpp"
#include <iostream>

PublicRecord::PublicRecord(std::string author, int key) : BaseRecord(author, key) {

}

int PublicRecord::calculateClearance() const {
    return 0;
}

