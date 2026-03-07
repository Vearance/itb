#include "Layla.hpp"
#include "Tigreal.hpp"

int main() {
    Layla* l1 = new Layla(10, 1000);
    l1->useSkill();

    Layla* l2 = new Layla(*l1);
    l2->heal(5);

    Layla* l3 = new Layla(50, 200);
    l3->moveTo(15, 30);
    *l3 = *l1;

    Tigreal t1(200, 10);
    t1.moveTo(5, 15);
    t1.taunt(10);

    Tigreal t2(t1);

    Tigreal t3(400, 20);
    t3.sacredHammer();

    Tigreal t4(t3);

    t1.useSkill();
    t3.useSkill();

    Tigreal t5(300, 15);
    t5.moveTo(10, 20);

    Tigreal* t6 = new Tigreal(t5);
    t6->taunt(5);

    delete t6;
    delete l1;
    delete l2;
    delete l3;

    return 0;
}
