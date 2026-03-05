#include "Penganan.hpp"
#include <iostream>

using namespace std;

int Penganan::uang = 0;
int Penganan::n_rumah = 0;

Penganan::Penganan() {
    // uang = 0;
    // n_rumah = 0;
    keik = 0;
    panekuk = 0;
}

Penganan::Penganan(int k, int p) {
    // uang = 0;
    // n_rumah = 0;
    keik = k;
    panekuk = p;
}

int Penganan::GetKeik() const {
    return keik;
}

int Penganan::GetPanekuk() const {
    return panekuk;
}

void Penganan::SetKeik(int k) {
    keik = k;
}

void Penganan::SetPanekuk(int k) {
    panekuk = k;
}

int Penganan::JumlahUang() {
    return uang;
}

int Penganan::HitungNRumah() {
    return n_rumah;
}

void Penganan::Print() {
    // 111keik-122panekuk
    cout << keik << "keik-" << panekuk << "panekuk";
    cout << "\n";
}

Penganan operator+ (const Penganan& a, const Penganan& b) {
    a.n_rumah++;
    return Penganan(a.GetKeik() + b.GetKeik(), a.GetPanekuk() + b.GetPanekuk());
}

Penganan operator- (const Penganan& a, const Penganan& b) {
    int keik, panekuk;
    if (a.GetKeik() >= b.GetKeik()) {
        // enough stock, sell what buyer wants
        keik = a.keik - b.keik;
        Penganan::uang += b.keik * 51;
    } else {
        // not enough stock, sell all
        keik = 0;
        Penganan::uang += a.keik * 51;
    }

    if (a.GetPanekuk() >= b.GetPanekuk()) {
        panekuk = a.GetPanekuk() - b.GetPanekuk();
        Penganan::uang += b.GetPanekuk() * 37;
    } else {
        panekuk = 0;
        Penganan::uang += a.GetPanekuk() * 37;
    }
    return Penganan(keik, panekuk);
}

Penganan operator^ (const Penganan& a, const int n) {
    int keik, panekuk;

    if (a.GetKeik() >= n) {
        // enough stock to donate
        keik = a.keik - n;
    } else {
        // not enough, pay for the shortage
        keik = 0;
        Penganan::uang -= (n - a.keik) * 51;
    }

    if (a.GetPanekuk() >= n) {
        panekuk = a.GetPanekuk() - n;
    } else {
        panekuk = 0;
        Penganan::uang -= (n - a.GetPanekuk()) * 37;
    }

    return Penganan(keik, panekuk);
}

Penganan operator^ (const int n, const Penganan& a) {
    return operator^(a, n);
}

// int main() {
//     cout << "hi";
// }