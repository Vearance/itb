#ifndef SAFE_BOX_EXCEPTION_HPP
#define SAFE_BOX_EXCEPTION_HPP

#include <exception>
#include <string>
#include <sstream>
using namespace std;

// Base class untuk semua exception SafeBox.
// Turunan dari std::exception override what() di setiap subclass.
class SafeBoxException : public exception {
public:
    // TODO: deklarasikan what() sebagai pure virtual
    virtual const char* what() const throw() = 0;
};


class BoxFullException : public SafeBoxException {
private:
    int maxCapacity;
    string message;

public:
    // TODO: konstruktor menerima kapasitas (int)
    // TODO: implementasi what() -> "Box penuh: kapasitas maks N"
    BoxFullException(int capacity) : maxCapacity(capacity) {
        ostringstream oss;
        oss << "Box penuh: kapasitas maks " << maxCapacity;
        message = oss.str();
    }

    const char* what() const throw() {
        return message.c_str();
    }
};


class BoxEmptyException : public SafeBoxException {
public:
    // TODO: implementasi what() -> "Box kosong"
    const char* what() const throw() {
        return "Box kosong";
    }
};


// Template: item yang tidak valid dapat bertipe apa saja (int, string, dll.)
// Gunakan ostringstream untuk mengonversi T ke string di dalam what().
// Contoh penggunaan ostringstream:
// int x = 42;
// ostringstream oss;
// oss << "Nilai x: " << x;
// string msg = oss.str(); // msg = "Nilai x: 42"
template<typename T>
class InvalidItemException : public SafeBoxException {
private:
    T invalidItem;
    string message;

public:
    // TODO: konstruktor menerima item (const T&)
    // TODO: implementasi what() -> "Item tidak valid: <item>"
    InvalidItemException(const T& item) : invalidItem(item) {
        ostringstream oss;
        oss << "Item tidak valid: " << invalidItem;
        message = oss.str();
    }

    const char* what() const throw() {
        return message.c_str();
    }
};

#endif
