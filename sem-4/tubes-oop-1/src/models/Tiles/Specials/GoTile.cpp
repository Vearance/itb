#include "GoTile.hpp"
#include "IGameContext.hpp"

using namespace std;

GoTile::GoTile(int id, const string& code, const string& name, int salary)
    : SpecialTile(id, code, name), salary(salary) {}

GoTile::~GoTile() {}

int GoTile::getSalary() const {
    return salary;
}

void GoTile::executeAction(IGameContext& ctx) {
    (void)ctx;
}
