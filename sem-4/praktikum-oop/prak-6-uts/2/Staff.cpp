#include "Staff.hpp"
#include "Formatter.hpp"

Staff::Staff(std::string name, int age, std::string contractEnd, std::string license, std::string dept)
    : ClubMember(name, age, contractEnd), coachingLicense(license), department(dept)
{
    // TODO: Constructor

}

double Staff::calculateWage() const {
    // TODO: return fixed wage, yaitu 50000.
    return 50000.0;
}

Staff::~Staff() {
    // TODO: log destruction menggunakan Formatter::log
    Formatter::log("~Staff", name, "session record freed.");
}
