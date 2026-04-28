#include "Exceptions.hpp"
#include "Game.hpp"
#include "GameView.hpp"
#include "Player.hpp"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

static void runEndScreen(GameView& view, const Game& game) {
    const auto& players = game.getPlayers();
    vector<const Player*> activePlayers;
    for (const auto& player : players) {
        if (player->getStatus() != PlayerStatus::BANKRUPT) {
            activePlayers.push_back(player.get());
        }
    }

    if (activePlayers.size() == 1) {
        vector<string> winners = {activePlayers.front()->getUsername()};
        vector<string> rankings;
        for (const auto& player : players) {
            rankings.push_back(player->getUsername() + " - cash M" + to_string(player->getMoney()) +
                               ", properti " + to_string(player->getProperties().size()) +
                               ", kartu " + to_string(player->getHand().size()));
        }
        view.showEndGameScreen(winners, rankings);
        return;
    }

    vector<const Player*> ranked;
    for (const auto& p : players) {
        ranked.push_back(p.get());
    }
    stable_sort(ranked.begin(), ranked.end(),
                [](const Player* a, const Player* b) { return *a > *b; });

    vector<string> winners;
    const Player* top = ranked.front();
    for (const Player* p : ranked) {
        if (!(*p < *top) && !(*top < *p)) {
            winners.push_back(p->getUsername());
        }
    }

    vector<string> rankings;
    for (int i = 0; i < static_cast<int>(ranked.size()); ++i) {
        rankings.push_back(to_string(i + 1) + ". " + ranked[i]->getUsername() + " - cash M" +
                           to_string(ranked[i]->getMoney()) + ", properti " +
                           to_string(ranked[i]->getProperties().size()) + ", kartu " +
                           to_string(ranked[i]->getHand().size()));
    }
    view.showEndGameScreen(winners, rankings);
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));
    GameView view;
    Game game;
    game.setUserInteraction(&view);

    while (true) {
        view.displayMainMenu();

        int choice = 0;
        string input;
        if (getline(cin, input)) {
            try {
                choice = stoi(input);
            } catch (...) {
                // choice remains 0, falls into invalid choice block
            }
        } else {
            cin.clear();
        }

        if (choice == 1) {
            // New game
            cout << "Pilih papan (contoh: config-60; ini perlu buat folder baru di root yg namanya "
                    "config-60) atau tekan enter untuk default (config): ";
            string configDir;
            getline(cin, configDir);
            if (configDir.empty() || configDir.find_first_not_of(" \t\r\n") == string::npos) {
                configDir = "config";
            }
            try {
                game.createGame(configDir);
            } catch (const GameException& e) {
                cout << "Gagal memulai permainan: " << e.what() << "\n";
                continue;
            } catch (const exception& e) {
                cout << "Gagal memulai permainan: " << e.what() << "\n";
                continue;
            }

            while (!game.checkWin()) {
                game.runCycle();
            }
            runEndScreen(view, game);

        } else if (choice == 2) {
            // Load game
            cout << "Nama file save: ";
            string filename;
            string tempVal;
            if (getline(cin, tempVal)) {
                istringstream iss(tempVal);
                iss >> filename;
            }

            if (filename.empty())
                continue;

            if (filename.find("data/") != 0) {
                filename = "data/" + filename;
            }

            try {
                game.loadGame(filename);
            } catch (const GameException& e) {
                cout << "Gagal memuat: " << e.what() << "\n";
                continue;
            } catch (const exception& e) {
                cout << "Gagal memuat: " << e.what() << "\n";
                continue;
            }

            while (!game.checkWin()) {
                game.runCycle();
            }
            runEndScreen(view, game);

        } else if (choice == 3) {
            cout << "Sampai jumpa!\n";
            break;
        } else {
            cout << "Pilihan tidak valid. Masukkan 1, 2, atau 3.\n";
        }
    }

    return 0;
}
