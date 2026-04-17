#include "ClubMember.hpp"
#include "Formatter.hpp"
#include <iostream>

ClubMember::ClubMember(string name, int age, string contractEnd) : name(name), age(age), contractEnd(contractEnd)
    // TODO: constructor
{
    
}

ClubMember::~ClubMember() {
    // TODO: log destruction menggunakan Formatter::log
    Formatter::log("~ClubMember", name, "contract record cleared.");
}   
