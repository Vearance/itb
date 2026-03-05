#include "Paper.hpp"
#include <iostream>

using namespace std;

int main() {
    Paper *a = new Paper('A');
    Paper b('B');
    Paper c('C');
    Paper d(c);
    a->fold();
    b.fold();
    c.fold();

    c.glue();
    // sampe sini bener
    // c.fold();
    // entah knpada fold(3)

    delete a;

    c.setName('X');

    d.fold();
    // fold(3)

    d.glue();
    // d.fold();



    return 0;
}