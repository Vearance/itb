#include "CommandHandler.hpp"
#include "Board.hpp"
#include "Deck.hpp"
#include "Dice.hpp"
#include "Exceptions.hpp"
#include "GameConfig.hpp"
#include "IGameContext.hpp"
#include "IUserInteraction.hpp"
#include "Logger.hpp"
#include "Player.hpp"
#include "PropertyManager.hpp"
#include "PropertyTile.hpp"
#include "SkillCard.hpp"
#include "Tile.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

void CommandHandler::dispatch(const string& input, Player& player, IGameContext& ctx,
                              PropertyManager& propMgr, Board& board, Dice& dice,
                              CardDeck<SkillCard>* skillDeck, IUserInteraction* ui, Logger& logger,
                              int& lastDiceTotal, SaveCallback saveCallback,
                              const vector<unique_ptr<Player>>& players) {
    if (input.empty())
        return;
    istringstream iss(input);
    string cmd;
    iss >> cmd;
    transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

    // Build raw pointer list for displayBoard
    vector<Player*> playerPtrs;
    for (const auto& p : players)
        playerPtrs.push_back(p.get());

    try {
        if (cmd == "CETAK_PAPAN") {
            if (ui)
                ui->displayBoard(board, playerPtrs, ctx.getCurrentTurn(), ctx.getConfig().maxTurn);

        } else if (cmd == "LEMPAR_DADU") {
            if (player.getHasRolledDice()) {
                throw GameStateException("Kamu sudah melempar dadu pada giliran ini.");
            } else {
                handleLemparDadu(player, dice, ctx, logger, lastDiceTotal, ui);
            }

        } else if (cmd == "ATUR_DADU") {
            if (player.getHasRolledDice()) {
                throw GameStateException("Kamu sudah melempar dadu pada giliran ini.");
            } else {
                int d1 = -1, d2 = -1;
                if (iss >> d1 >> d2) {
                    handleLemparDadu(player, dice, ctx, logger, lastDiceTotal, ui, d1, d2);
                } else {
                    if (ui)
                        ui->printMessage("Format: ATUR_DADU X Y (contoh: ATUR_DADU 3 4)\n");
                }
            }

        } else if (cmd == "CETAK_AKTA") {
            string code;
            if (!(iss >> code)) {
                if (ui) {
                    ui->printMessage("Masukkan kode petak: ");
                    code = ui->readLine();
                }
            }
            Tile* t = board.getTileByCode(code);
            auto* prop = dynamic_cast<PropertyTile*>(t);
            if (prop) {
                if (ui)
                    ui->printPropertyDeed(*prop);
            } else {
                throw InvalidPropertyException("Petak \"" + code +
                                               "\" tidak ditemukan atau bukan properti.");
            }

        } else if (cmd == "CETAK_PROPERTI") {
            if (ui)
                ui->printPlayerProperties(player);

        } else if (cmd == "GADAI") {
            propMgr.mortgage(player, board, ctx.getCurrentTurn(), logger, ui);

        } else if (cmd == "TEBUS") {
            propMgr.redeem(player, ctx.getCurrentTurn(), logger, ui);

        } else if (cmd == "BANGUN") {
            propMgr.build(player, board, ctx.getCurrentTurn(), logger, ui);

        } else if (cmd == "GUNAKAN_KEMAMPUAN") {
            handleGunakanKemampuan(player, ctx, skillDeck, logger, ui);

        } else if (cmd == "CETAK_LOG") {
            int n = -1;
            iss >> n;
            auto logs = logger.getRecentLogs(n);
            if (ui)
                ui->printTransactionLogs(logs);

        } else if (cmd == "SIMPAN") {
            if (player.getHasRolledDice()) {
                throw GameStateException(
                    "SIMPAN hanya bisa dilakukan di awal giliran (sebelum melempar dadu).");
            } else {
                string filename;
                if (!(iss >> filename))
                    filename = "nimonspoli_save.txt";

                if (filename.find("data/") != 0) {
                    filename = "data/" + filename;
                }
                {
                    ifstream checkFile(filename);
                    if (checkFile.good()) {
                        checkFile.close();
                        if (ui) {
                            ui->printMessage("File '" + filename +
                                             "' sudah ada. Timpa file lama? (y/n): ");
                            string response = ui->readLine();
                            if (response.empty() || (response[0] != 'y' && response[0] != 'Y')) {
                                ui->printMessage("Penyimpanan dibatalkan.\n");
                                return;
                            }
                        }
                    }
                }
                saveCallback(filename);
                if (ui)
                    ui->printMessage("Permainan berhasil disimpan ke: " + filename + "\n");
                logger.logEvent(LogLevel::INFO, ctx.getCurrentTurn(), player.getUsername(),
                                "SIMPAN", "Simpan ke " + filename);
            }

        } else {
            throw InvalidCommandException(
                "Perintah tidak dikenali: " + cmd +
                ". Ketik CETAK_PAPAN, LEMPAR_DADU, ATUR_DADU X Y, CETAK_AKTA, "
                "CETAK_PROPERTI, GADAI, TEBUS, BANGUN, GUNAKAN_KEMAMPUAN, "
                "CETAK_LOG [N], SIMPAN [FILE].");
        }
    } catch (const GameException& e) {
        if (ui)
            ui->printMessage(string(e.what()) + "\n");
    }
}

void CommandHandler::handleLemparDadu(Player& player, Dice& dice, IGameContext& ctx, Logger& logger,
                                      int& lastDiceTotal, IUserInteraction* ui, int d1, int d2) {
    try {
        if (d1 < 0) {
            lastDiceTotal = dice.rollRandom();
        } else {
            lastDiceTotal = dice.rollManual(d1, d2);
        }
    } catch (const exception& e) {
        if (ui)
            ui->printMessage(string("Error dadu: ") + e.what() + "\n");
        return;
    }

    player.setRolledDice(true);
    auto dv = dice.getDiceValues();
    if (ui) {
        ui->printMessage("\nMengocok dadu...\n");
        ui->printMessage("Hasil: " + to_string(dv.first) + " + " + to_string(dv.second) + " = " +
                         to_string(lastDiceTotal) + "\n");
    }

    logger.logEvent(LogLevel::INFO, ctx.getCurrentTurn(), player.getUsername(), "DADU",
                    "Lempar: " + to_string(dv.first) + "+" + to_string(dv.second) + "=" +
                        to_string(lastDiceTotal));

    if (dice.isSpeedingViolation()) {
        if (ui)
            ui->printMessage(player.getUsername() +
                             " melanggar batas kecepatan (3 double berturut)! Masuk Penjara!\n");
        ctx.sendPlayerToJail(player);
        logger.logEvent(LogLevel::INFO, ctx.getCurrentTurn(), player.getUsername(), "PENJARA",
                        "Speeding violation - masuk penjara");
        return;
    }

    if (ui)
        ui->printMessage("Memajukan bidak " + player.getUsername() + " sebanyak " +
                         to_string(lastDiceTotal) + " petak...\n");
    ctx.movePlayerBy(player, lastDiceTotal);
}

void CommandHandler::handleGunakanKemampuan(Player& player, IGameContext& ctx,
                                            CardDeck<SkillCard>* skillDeck, Logger& logger,
                                            IUserInteraction* ui) {
    if (player.getHasUsedSkillThisTurn()) {
        throw GameStateException("Penggunaan kartu dibatasi maksimal 1 kali dalam 1 giliran.");
    }
    if (player.getHasRolledDice()) {
        throw GameStateException("Kartu kemampuan hanya bisa digunakan SEBELUM melempar dadu.");
    }
    const auto& hand = player.getHand();
    if (hand.empty()) {
        throw GameStateException("Kamu tidak memiliki kartu kemampuan.");
    }

    if (ui) {
        ui->printMessage("Daftar Kartu Kemampuan Spesial Anda:\n");
        for (int i = 0; i < static_cast<int>(hand.size()); i++) {
            ui->printMessage(to_string(i + 1) + ". " + hand[i]->getName() + " - " +
                             hand[i]->getDescription() + "\n");
        }
        ui->printMessage("0. Batal\n");
        ui->printMessage("Pilih kartu (0-" + to_string(hand.size()) + "): ");
    }

    int choice = 0;
    if (player.getIsComputer()) {
        choice = (rand() % hand.size()) + 1;
        if (ui)
            ui->printMessage(to_string(choice) + "\n");
    } else {
        if (ui)
            choice = ui->readInt();
    }
    if (choice < 1 || choice > static_cast<int>(hand.size())) {
        if (choice == 0)
            return;
        throw InvalidCommandException("Nomor kartu tidak valid.");
    }

    SkillCard* card = hand[choice - 1];
    string cardName = card->getName();
    player.removeCard(card);
    card->executeAction(ctx);
    skillDeck->discard(card);
    player.setUsedSkillThisTurn(true);
    logger.logEvent(LogLevel::INFO, ctx.getCurrentTurn(), player.getUsername(), "KARTU",
                    "Gunakan " + cardName);
}
