#ifndef CARGO_BAY_HPP
#define CARGO_BAY_HPP

#include "Item.hpp"
#include "CargoException.hpp"

template <class T>
class CargoBay {
private:
    Item<T>* arr;
    int capacity;
    int size;

public:
    // TODO Constructor & Destructor
    // Constructor: inisialisasi capacity, size = 0, alokasi array arr.
    // Destructor: Dealokasi array arr.
    CargoBay(int cap);
    ~CargoBay();

    // TODO Push & Pop (Standard)
    // push: Lempar CargoFullException jika size >= capacity. Taruh item ke index terakhir array
    // pop: Lempar CargoEmptyException jika size <= 0. Return item terakhir (LIFO) dan update size.
    void push(Item<T> item);
    Item<T> pop();

    // TODO extractItem
    // Cari item PERTAMA (dimulai dari index 0 hingga size-1)
    // yang nilai `data`-nya SAMA DENGAN `targetData`.
    // Jika ketemu:
    //     - Return Item tersebut.
    //     - Hapus Item tersebut dari array,
    //       lalu geser seluruh Item di kanannya ke kiri agar array tidak bolong.
    //     - Update size.
    // Jika tidak ketemu, lempar ItemNotFoundException.
    Item<T> extractItem(T targetData);

    // TODO forge
    // Menempa (Menggabungkan) dua Item teratas menjadi satu Item baru.
    // Cek apakah size < 2. Jika ya, lempar NotEnoughItemsException.
    // Jika aman, keluarkan (pop) item pertama dan item kedua.
    // Buatlah data baru hasil penjumlahan berurutan: `baseData` + data Top 1 + data Top 2.
    //   (Contoh: Jika baseData = "X", Top 1 = "Y", Top 2 = "Z", maka data baru = "XYZ").
    // Dipastikan tipe data T support operator+
    // Buatlah durability baru: rata-rata dari durability item pertama dan kedua (integer division).
    // Buat objek Item baru dengan data & durability baru tersebut, lalu push() ke CargoBay.
    void forge(T baseData);

    // TODO transferTo
    // Pindahkan sejumlah `count` item dari atas cargo `this` ke cargo `other`.
    // PROSES TRANSFER dilakukan satu per satu (LIFO).
    // Perhatikan bahwa urutan item akan terbalik.
    // Jika, size < count, lempar NotEnoughItemsException
    // PENANGANAN EXCEPTION:
    //    - Jika other ternyata penuh di tengah jalan (melempar CargoFullException),
    //      TANGKAP error tersebut dengan try-catch!
    //    - Kembalikan 1 item yang gagal masuk itu ke cargo `this`.
    //    - Lempar TransferFailedException ke luar fungsi untuk memberitahu bahwa transfer terhenti.
    void transferTo(CargoBay<T>& other, int count);
};

// Implementasi di sini (atau langsung di deklarasi methods juga bisa sih)

template <class T>
CargoBay<T>::CargoBay(int cap) {
    this->capacity = cap;
    this->size = 0;
    this->arr = new Item<T>[cap];
}

template <class T>
CargoBay<T>::~CargoBay() {
    delete[] this->arr;
}

template <class T>
void CargoBay<T>::push(Item<T> item) {
    if (this->size >= this->capacity) {
        throw CargoFullException();
    }

    this->arr[this->size] = item;
    this->size++;
}

template <class T>
Item<T> CargoBay<T>::pop() {
    if (this->size <= 0) {
        throw CargoEmptyException();
    }

    this->size--;
    return this->arr[this->size];
}

template <class T>
Item<T> CargoBay<T>::extractItem(T targetData) {
    for (int i = 0; i < this->size; i++) {
        if (this->arr[i].getData() == targetData) {
            Item<T> extracted = this->arr[i];
            for (int j = i; j < this->size - 1; j++) {
                this->arr[j] = this->arr[j + 1];
            }
            this->size--;
            return extracted;
        }
    }

    throw ItemNotFoundException();
}

template <class T>
void CargoBay<T>::forge(T baseData) {
    if (this->size < 2) {
        throw NotEnoughItemsException();
    }

    Item<T> firstTop = this->pop();
    Item<T> secondTop = this->pop();

    T forgedData = baseData + firstTop.getData() + secondTop.getData();
    int forgedDurability = (firstTop.getDurability() + secondTop.getDurability()) / 2;

    this->push(Item<T>(forgedData, forgedDurability));
}

template <class T>
void CargoBay<T>::transferTo(CargoBay<T>& other, int count) {
    if (this->size < count) {
        throw NotEnoughItemsException();
    }

    for (int i = 0; i < count; i++) {
        Item<T> movedItem = this->pop();
        try {
            other.push(movedItem);
        } catch (const CargoFullException&) {
            this->push(movedItem);
            throw TransferFailedException();
        }
    }
}

#endif
