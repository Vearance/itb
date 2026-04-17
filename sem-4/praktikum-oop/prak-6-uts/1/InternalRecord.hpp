#ifndef INTERNAL_RECORD_HPP
#define INTERNAL_RECORD_HPP

#include "BaseRecord.hpp"

class InternalRecord : protected BaseRecord {
    public:
        InternalRecord(std::string author, int key);
        // int calculateClearance() const override { return getKey() % 10; }
        int peekSecurity();
};

#endif
