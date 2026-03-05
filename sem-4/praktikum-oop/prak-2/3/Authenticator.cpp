#include "Authenticator.hpp"

Authenticator::Authenticator() : userCount(0), secretResource("Ini adalah data rahasia!") {}

void Authenticator::registerUser(const string &username, const string &password, bool isAdmin) {
    if(Authenticator::userCount < MAX_USERS) {
        bool found = false;
        int i = 0;
        while (!found && i < Authenticator::userCount) {
            if(Authenticator::users[i]->getUsername() == username) {
                found = true;
            }
            i++;
        }

        if(!found) {
            cout << "Sukses: User " << username << " berhasil terdaftar!\n";
            if (isAdmin) {
                users[Authenticator::userCount] = new AdminUser(username, password);
            }
            else {
                users[Authenticator::userCount] = new User(username, password);
            }
            userCount++;
        }
        else {
            cout <<"Gagal: Username sudah digunakan!\n";
        }
    }
    else {
        cout <<"Gagal: Kapasitas user penuh!\n";
    }
}

void Authenticator::login(const string &username, const string &password) {
    bool found = false;
    int i = 0;
    while (!found && i < Authenticator::userCount) {
        if(Authenticator::users[i]->getUsername() == username && Authenticator::users[i]->checkPassword(password)) {
            found = true;
        }
        i++;
    }
    
    if(!found) {
        cout << "Gagal: Username atau password salah!\n";
    }
    else {
        currentUser = Authenticator::users[i-1];
        cout << "Sukses: Login berhasil! Selamat datang, " << username << ".\n";
    }
}

void Authenticator::logout() {
    if(currentUser) {
        cout << "Sukses: User " << currentUser->getUsername() << " telah logout.\n";
        currentUser = nullptr;
    }
    else {
        cout << "Gagal: Tidak ada user yang sedang login!\n";
    }
}

void Authenticator::accessResource() const {
    if(!currentUser) {
        cout << "Akses ditolak! Silakan login terlebih dahulu.\n";
    }
    else {
        cout <<"Resource: " << Authenticator::secretResource << "\n";
    }
}

void Authenticator::setResource(const string &newResource) {
    if(!currentUser) {
        cout << "Gagal: Tidak bisa mengubah resource! Silakan login terlebih dahulu.\n";
    }
    else {
        if (currentUser->isAdmin()) {
            Authenticator::secretResource = newResource;
            cout << "Sukses: Resource telah diperbarui oleh " << currentUser->getUsername() << ".\n";
        }
        else {
            cout << "Gagal: Tidak bisa mengubah resource! Hanya admin yang dapat melakukan ini.\n";
        }
    }

}

Authenticator::~Authenticator() {
    for(int i = 0; i < Authenticator::userCount; i++) {
        delete Authenticator::users[i];
    }
}