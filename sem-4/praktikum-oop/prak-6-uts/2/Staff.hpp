#pragma once
#include "ClubMember.hpp"
#include <string>

class Staff : virtual public ClubMember
{
protected:
    // TODO: deklarasikan atribut kelas (coachingLicense, department)
    std::string coachingLicense;
    std::string department;

public:
    // TODO: deklarasikan constructor, methods calculateWage() dan pure virtual getSpeciality(), serta destructor
    Staff(std::string name, int age, std::string contractEnd, std::string license, std::string dept);
    virtual ~Staff();
    double calculateWage() const override;
    virtual std::string getSpecialty() const = 0;
};
