#include "MesinHitungNeroifa.hpp"
#include "MesinHitungException.hpp"

#include <stack>
#include <sstream>
#include <cctype>
#include <string>
using namespace std;

MesinHitungNeroifa::MesinHitungNeroifa() {}

int MesinHitungNeroifa::hitung(const string& ekspresi) const {
    stack<int> st;
    stringstream ss(ekspresi);
    string token;

    while (ss >> token) {
        // TODO:
        // Jika token adalah operator (+, -, *, /),
        // lakukan operasi terhadap dua elemen teratas stack.
        // Jika jumlah operand tidak mencukupi, lempar exception yang sesuai.

        if (token == "+" || token == "-" || token == "*" || token == "/") {
            // TODO: cek apakah stack memiliki minimal 2 operand
            if(st.size() < 2) {
                throw InsufficientOperandException();
            }
            // TODO: ambil dua operand teratas dari stack
            int operand2 = st.top();
            st.pop();
            int operand1 = st.top();
            st.pop();

            // TODO: lakukan operasi sesuai token
            if(token == "/" && operand2 == 0) {
                throw DivisionByZeroException();
            }
            if(token == "+") st.push(operand1 + operand2) ;
            if(token == "-") st.push( operand1 - operand2);
            if(token == "*") st.push( operand1 * operand2);
            if(token == "/") st.push( operand1 / operand2);

            // Perhatikan bahwa pembagian dengan nol harus melempar exception
        } else {
            // TODO:
            // Anggap token sebagai kandidat bilangan bulat.
            // Cek apakah token valid:
            // - boleh berupa bilangan negatif
            // - selain itu semua karakter harus digit
            bool valid = true;
            int startIdx = 1;

            // TODO: tangani kasus bilangan negatif
            if(&token[0] == "-") {
                for(int i = startIdx ; i < token.size(); i++) {
                    if(!std::isdigit(token[i])) {
                        valid = false;
                        // throw InvalidTokenException(&token[i]);
                    }
                }
            }
            else {
                for(int i = startIdx ; i < token.size(); i++) {
                    if(!std::isdigit(token[i])) {
                        valid = false;
                        // throw InvalidTokenException(&token[i]);
                    }
                }
            }
            
            

            

            // TODO: cek apakah semua karakter yang relevan adalah digit

            if (!valid) {
               throw InvalidTokenException(token);
            }

            // TODO: ubah token menjadi integer lalu push ke stack
            stringstream tokenbaru(token);
            int angka;
            tokenbaru >> angka;
            st.push(angka);
        }
    }

    // TODO:
    // Setelah semua token diproses, stack harus berisi tepat satu elemen.
    // Jika tidak, lempar exception yang sesuai.
    if(st.size() != 1) {
        throw InvalidExpressionException();
    }

    // TODO: kembalikan hasil akhir
    return st.top();
}