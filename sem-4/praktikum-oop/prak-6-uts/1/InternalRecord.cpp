#include "InternalRecord.hpp"
#include <iostream>

InternalRecord::InternalRecord(std::string author, int key) : BaseRecord(author, key) {

}

int InternalRecord::peekSecurity() {
    return calculateClearance() * 2;
}

