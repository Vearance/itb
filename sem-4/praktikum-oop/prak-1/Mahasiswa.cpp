#include <iostream>
#include <string>
#include <cstdio>

using namespace std;

class Mahasiswa {
private:
    string name;
    string nim;
    int total_sks;
    float ipk;

public:
    static int counter;

    Mahasiswa() {
        name = "NPC";
        nim = "135" + to_string(counter);
        total_sks = 0;
        ipk = 0.0f;
        counter++;
    }

    Mahasiswa(string nama) {
        if (nama == "") {
            name = "NPC";
        } else {
            name = nama;
        }
        nim = "135" + to_string(counter);
        total_sks = 0;
        ipk = 0.0f;
        counter++;
    }

    void tambahNilai(int sks_baru, float nilai) {
        if (total_sks + sks_baru > 0) {
            ipk = (ipk * total_sks + nilai * sks_baru) / (total_sks + sks_baru);
            total_sks += sks_baru;
        }
    }

    void info() {
        printf("INFORMASI MAHASISWA\n");
        printf("Nama: %s\n", name.c_str());
        printf("NIM: %s\n", nim.c_str());
        printf("IPK: %.2f\n", ipk);
        printf("SKS: %d\n", total_sks);
    }
};

int Mahasiswa::counter = 0;