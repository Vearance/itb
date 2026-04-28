#include "Exceptions.hpp"
#include <iostream>

using namespace std;

InsufficientFundsException::InsufficientFundsException(int required, int available)
    : GameException("Uang kamu tidak cukup! Harga: M" + to_string(required) +
                    " | Uang kamu saat ini: M" + to_string(available)) {}

InvalidPropertyException::InvalidPropertyException(const string& reason) : GameException(reason) {}

CardLimitException::CardLimitException()
    : GameException("PERINGATAN: Kamu sudah memiliki 3 kartu di tangan (Maksimal 3)! "
                    "Kamu diwajibkan membuang 1 kartu.") {}

InvalidCommandException::InvalidCommandException(const string& command) : GameException(command) {}

FileException::FileException(const string& filename, const string& operation)
    : GameException([&]() -> string {
          if (operation == "simpan") {
              return "Gagal menyimpan file! Pastikan direktori dapat ditulis.";
          } else if (operation == "tidak_ditemukan") {
              return "File \"" + filename + "\" tidak ditemukan.";
          } else if (operation == "muat") {
              return "Gagal memuat file! File rusak atau format tidak dikenali.";
          }
          return "Kesalahan file \"" + filename + "\": " + operation;
      }()) {}

GameStateException::GameStateException(const string& reason) : GameException(reason) {}
