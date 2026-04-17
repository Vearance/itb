#pragma once
#include <string>
using namespace std;
class ClubMember {
protected:
    // TODO: deklarasikan atribut kelas (name, age, contractEnd)
    string name;
    int age;
    string contractEnd;


public:
    // TODO: deklarasikan constructor, methods getProfile(), calculateWage(), work(), serta destructor
    ClubMember(string name, int age, string contractEnd);
    virtual ~ClubMember();
    virtual string getProfile() const = 0;
    virtual double calculateWage() const = 0;
    virtual void work() const = 0;

};
