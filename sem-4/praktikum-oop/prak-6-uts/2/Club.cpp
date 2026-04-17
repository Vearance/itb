#include "Club.hpp"
#include "Player.hpp"
#include "Staff.hpp"
#include <iostream>

using namespace std;

Club::Club(std::string name) : clubName(name)
    // TODO: constructor
{


}

void Club::addMember(ClubMember* member) {
    // TODO: push member ke roster
    this->roster.push_back(member);
}

void Club::printSquadReport() const {
    std::cout << "\n======================================\n";
    std::cout << "  " << clubName << " - Squad Report\n";
    std::cout << "======================================\n";
    // TODO: panggil getProfile() dari setiap member
    for (const auto* member : roster) {
        std::cout << member->getProfile() << '\n';
    }
}

void Club::runTraining() const {
    std::cout << "\n--- Match Day Actions ---\n";
    // TODO: panggil work() dari setiap member
    for (const auto* member : roster) {
        member->work();
    }
}

Club::~Club() {
    std::cout << "\n--- Releasing Club Roster ---\n";
    // TODO: delete semua ClubMember* di roster
    for (auto* member : roster) {
        delete member;
    }
}
