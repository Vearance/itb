#include "GameView.hpp"
#include "ColorGroup.hpp"
#include "Logger.hpp"
#include "StreetTile.hpp"
#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <cstdlib>
#endif

using namespace std;

GameView::GameView() {}

GameView::~GameView() {}

// ── Generic output ────────────────────────────────────────────────────────────

void GameView::printMessage(const std::string& msg) {
    cout << msg;
}

void GameView::printEmptyLine() {
    cout << "\n";
}

string GameView::readLine() {
    string line;
    if (!getline(cin, line)) {
        cin.clear();
    }
    return line;
}

int GameView::readInt() {
    string line = readLine();
    try {
        return stoi(line);
    } catch (...) {
        return -1;
    }
}

void GameView::clearScreen() const {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void GameView::displayMainMenu() const {
    cout << "============================================\n";
    cout << "           N I M O N S P O L I             \n";
    cout << "============================================\n";
    cout << "  1. Mulai Permainan Baru\n";
    cout << "  2. Muat Permainan Tersimpan\n";
    cout << "  3. Keluar\n";
    cout << "============================================\n";
    cout << "Pilih (1/2/3): ";
}

vector<pair<string, bool>> GameView::promptPlayerSetup() {
    int count = 0;
    while (true) {
        cout << "Jumlah pemain (2-4): ";
        count = readInt();
        if (count >= 2 && count <= 4) {
            break;
        }
        cout << "Input tidak valid. Jumlah pemain harus antara 2 hingga 4.\n";
    }

    vector<pair<string, bool>> results(count);
    for (int i = 0; i < count; ++i) {
        string currentName;
        while (true) {
            cout << "Nama pemain " << (i + 1) << ": ";
            currentName = readLine();
            if (currentName.empty() || currentName.find_first_not_of(" \t\r\n") == string::npos) continue;
            
            istringstream iss(currentName);
            iss >> currentName;

            bool duplicate = false;
            for (int j = 0; j < i; ++j) {
                if (results[j].first == currentName) {
                    duplicate = true;
                    break;
                }
            }
            if (duplicate) {
                cout << "Nama " << currentName << " sudah digunakan. Masukkan nama lain.\n";
            } else {
                break;
            }
        }

        bool isCom = false;
        while (true) {
            cout << "Apakah " << (i + 1) << " sebuah COM? (y/n) ";
            string ans = readLine();
            if (ans == "y" || ans == "Y") {
                isCom = true;
                break;
            } else if (ans == "n" || ans == "N") {
                isCom = false;
                break;
            } else {
                cout << "Input tidak valid. Masukkan y atau n.\n";
            }
        }
        results[i] = make_pair(currentName, isCom);
    }
    return results;
}

void GameView::displayBoard(const Board& board, const vector<Player*>& players, int currentTurn,
                            int maxTurn) const {
    int totalTiles = board.getTotalTiles();
    if (totalTiles == 0) {
        cout << "[Board kosong]\n";
        return;
    }

    const int CW = 10;
    // Smallest S where 4*(S-1) >= totalTiles
    const int S = (totalTiles + 3) / 4 + 1;
    const int COLS = S;
    const int INNER_W = (S - 2) * (CW + 1) - 1;

    // ── ANSI colour helpers ─────────────────────────────────────────────────────
    auto ansi = [](const string& cc) -> string {
        if (cc == "CK") {
            return "\033[33m"; // brown → yellow
        }
        if (cc == "BM") {
            return "\033[96m"; // light blue → bright cyan
        }
        if (cc == "PK") {
            return "\033[95m"; // pink → bright magenta
        }
        if (cc == "OR") {
            return "\033[38;5;208m"; // orange
        }
        if (cc == "MR") {
            return "\033[91m"; // red
        }
        if (cc == "KN") {
            return "\033[93m"; // yellow
        }
        if (cc == "HJ") {
            return "\033[92m"; // green
        }
        if (cc == "BT") {
            return "\033[94m"; // dark blue
        }
        if (cc == "AB") {
            return "\033[97m"; // utility → bright white
        }
        return "\033[37m"; // DF → white
    };
    const string RST = "\033[0m";

    // ── Helpers ─────────────────────────────────────────────────────────────────
    auto pad = [](const string& s, int w) -> string {
        if (static_cast<int>(s.size()) >= w)
            return s.substr(0, w);
        return s + string(w - static_cast<int>(s.size()), ' ');
    };

    auto centerStr = [](const string& s, int w) -> string {
        if (static_cast<int>(s.size()) >= w)
            return s.substr(0, w);
        int lp = (w - static_cast<int>(s.size())) / 2;
        string r = string(lp, ' ') + s;
        r += string(w - static_cast<int>(r.size()), ' ');
        return r;
    };

    // ── Position mapping: generalized for any S-side grid ───────────────────────
    // Perimeter clockwise from bottom-right (pos 0 = GO at bottom-right corner).
    auto getPos = [S](int row, int col) -> int {
        if (row == S - 1)
            return (S - 1) - col;       // bottom row, right-to-left
        if (col == 0)
            return 2 * (S - 1) - row;   // left col, top-to-bottom
        if (row == 0)
            return 2 * (S - 1) + col;   // top row, left-to-right
        return 3 * (S - 1) + row;       // right col, top-to-bottom
    };

    // ── Colour-group code for tile ──────────────────────────────────────────────
    auto getCC = [](Tile* t) -> string {
        if (auto* s = dynamic_cast<const StreetTile*>(t)) {
            switch (s->getColorGroup()) {
            case ColorGroup::BROWN:
                return "CK";
            case ColorGroup::LIGHT_BLUE:
                return "BM";
            case ColorGroup::PINK:
                return "PK";
            case ColorGroup::ORANGE:
                return "OR";
            case ColorGroup::RED:
                return "MR";
            case ColorGroup::YELLOW:
                return "KN";
            case ColorGroup::GREEN:
                return "HJ";
            case ColorGroup::DARK_BLUE:
                return "BT";
            }
        }
        if (auto* p = dynamic_cast<const PropertyTile*>(t))
            if (p->getType() == PropertyType::UTILITY)
                return "AB";
        return "DF";
    };

    int jailPos = board.getJailPosition();

    // ── Print cell Line 1: [CC] NNN ─────────────────────────────────────────────
    auto printL1 = [&](int pos) {
        Tile* t = board.getTileAt(pos);
        if (!t) {
            cout << string(CW, ' ');
            return;
        }
        string cc = getCC(t);
        string code = t->getCode();
        while (static_cast<int>(code.size()) < 3)
            code += ' ';
        code = code.substr(0, 3);
        cout << ansi(cc) << "[" << cc << "]" << RST << " " << code << string(max(0, CW - 8), ' ');
    };

    // ── Print cell Line 2: ownership / building / player tokens ─────────────────
    auto printL2 = [&](int pos) {
        Tile* t = board.getTileAt(pos);
        if (!t) {
            cout << string(CW, ' ');
            return;
        }
        string content;
        if (pos == jailPos) {
            string inIds, visIds;
            for (const auto* p : players) {
                if (p->getStatus() == PlayerStatus::BANKRUPT)
                    continue;
                if (p->getPosition() != jailPos)
                    continue;
                if (p->isInJail())
                    inIds += to_string(p->getId() + 1);
                else
                    visIds += to_string(p->getId() + 1);
            }
            if (!inIds.empty())
                content += "IN:" + inIds;
            if (!visIds.empty()) {
                if (!content.empty())
                    content += " ";
                content += "V:" + visIds;
            }
        } else {
            auto* prop = dynamic_cast<PropertyTile*>(t);
            if (prop && prop->getOwner()) {
                content += "P" + to_string(prop->getOwner()->getId() + 1);
                if (auto* st = dynamic_cast<const StreetTile*>(prop)) {
                    int lvl = st->getPropertyLevel();
                    if (lvl == 5)
                        content += " *";
                    else if (lvl > 0)
                        content += " " + string(lvl, '^');
                }
            }
            for (const auto* p : players) {
                if (p->getStatus() == PlayerStatus::BANKRUPT)
                    continue;
                if (p->getPosition() != pos)
                    continue;
                string mk = "(" + to_string(p->getId() + 1) + ")";
                if (!content.empty() &&
                    static_cast<int>(content.size()) + static_cast<int>(mk.size()) + 1 <= CW)
                    content += " ";
                content += mk;
            }
        }
        cout << pad(content, CW);
    };

    // ── Separator lines ─────────────────────────────────────────────────────────
    auto printFullSep = [&]() {
        cout << "+";
        for (int c = 0; c < COLS; ++c)
            cout << string(CW, '-') << "+";
        cout << "\n";
    };

    // ── Interior content ────────────────────────────────────────────────────────
    // (S-2) interior rows × 2 content lines + (S-3) separator lines
    const int numInteriorRows = S - 2;
    const int contentSlots = numInteriorRows * 2;
    const int sepSlots = max(0, numInteriorRows - 1);
    vector<string> interior(contentSlots + sepSlots, pad("", INNER_W));

    string titleBox = string(34, '=');
    string titleTxt = "||          NIMONSPOLI          ||";
    string divider = string(34, '-');
    string turnStr =
        currentTurn >= 0 ? "TURN " + to_string(currentTurn) + " / " + to_string(maxTurn) : "";

    if (numInteriorRows == 9) {
        // ── Standard 40-tile layout: exact original assignment ────────────────
        // Content slots [0..17] — 9 rows × 2 lines
        interior[0]  = pad("", INNER_W);
        interior[1]  = pad("", INNER_W);
        interior[2]  = centerStr(titleBox, INNER_W);
        interior[3]  = centerStr(titleTxt, INNER_W);
        interior[4]  = pad("", INNER_W);
        interior[5]  = centerStr(turnStr, INNER_W);
        interior[6]  = pad("", INNER_W);
        interior[7]  = centerStr(divider, INNER_W);
        interior[8]  = centerStr("P1-P4 : Properti milik Pemain 1-4", INNER_W);
        interior[9]  = centerStr("^     : Rumah Level 1", INNER_W);
        interior[10] = centerStr("^^^   : Rumah Level 3", INNER_W);
        interior[11] = centerStr("* : Hotel (Maksimal)", INNER_W);
        interior[12] = centerStr(divider, INNER_W);
        interior[13] = centerStr("KODE WARNA:", INNER_W);
        interior[14] = centerStr("[BM]=Biru Muda [KN]=Kuning", INNER_W);
        interior[15] = centerStr("[PK]=Pink      [HJ]=Hijau", INNER_W);
        interior[16] = centerStr("[DF]=Aksi      [AB]=Utilitas", INNER_W);
        interior[17] = pad("", INNER_W);
        // Separator slots [18..25] — text that floats in the partial-sep lines
        interior[18] = pad("", INNER_W);
        interior[19] = centerStr(titleBox, INNER_W);
        interior[20] = pad("", INNER_W);
        interior[21] = centerStr("LEGENDA KEPEMILIKAN & STATUS", INNER_W);
        interior[22] = centerStr("^^    : Rumah Level 2", INNER_W);
        interior[23] = centerStr("(1)-(4): Bidak (IN=Tahanan, V=Mampir)", INNER_W);
        interior[24] = centerStr("[CK]=Coklat    [MR]=Merah", INNER_W);
        interior[25] = centerStr("[OR]=Orange    [BT]=Biru Tua", INNER_W);
    } else {
        // ── Dynamic layout: sequential fill ───────────────────────────────────
        vector<string> legend;
        legend.push_back(pad("", INNER_W));
        legend.push_back(centerStr(titleBox, INNER_W));
        legend.push_back(centerStr(titleTxt, INNER_W));
        legend.push_back(centerStr(titleBox, INNER_W));
        legend.push_back(pad("", INNER_W));
        legend.push_back(centerStr(turnStr, INNER_W));
        legend.push_back(pad("", INNER_W));
        legend.push_back(centerStr(divider, INNER_W));
        legend.push_back(centerStr("LEGENDA KEPEMILIKAN & STATUS", INNER_W));
        legend.push_back(centerStr("P1-P4 : Properti milik Pemain 1-4", INNER_W));
        legend.push_back(centerStr("^     : Rumah Level 1", INNER_W));
        legend.push_back(centerStr("^^    : Rumah Level 2", INNER_W));
        legend.push_back(centerStr("^^^   : Rumah Level 3", INNER_W));
        legend.push_back(centerStr("* : Hotel (Maksimal)", INNER_W));
        legend.push_back(centerStr("(1)-(4): Bidak (IN=Tahanan, V=Mampir)", INNER_W));
        legend.push_back(centerStr(divider, INNER_W));
        legend.push_back(centerStr("KODE WARNA:", INNER_W));
        legend.push_back(centerStr("[CK]=Coklat    [MR]=Merah", INNER_W));
        legend.push_back(centerStr("[BM]=Biru Muda [KN]=Kuning", INNER_W));
        legend.push_back(centerStr("[PK]=Pink      [HJ]=Hijau", INNER_W));
        legend.push_back(centerStr("[OR]=Orange    [BT]=Biru Tua", INNER_W));
        legend.push_back(centerStr("[DF]=Aksi      [AB]=Utilitas", INNER_W));
        legend.push_back(pad("", INNER_W));
        for (int i = 0; i < static_cast<int>(interior.size()) &&
                        i < static_cast<int>(legend.size());
             ++i) {
            interior[i] = legend[i];
        }
    }

    // ── Render board ────────────────────────────────────────────────────────────
    for (int row = 0; row < S; ++row) {
        if (row == 0 || row == 1 || row == S - 1) {
            printFullSep();
        } else {
            int sepIdx = contentSlots + (row - 2);
            cout << "+" << string(CW, '-') << "+" << interior[sepIdx] << "+"
                 << string(CW, '-') << "+\n";
        }

        // Line 1 (tile code + colour)
        cout << "|";
        if (row == 0 || row == S - 1) {
            for (int col = 0; col < S; ++col) {
                int pos = getPos(row, col);
                if (pos < totalTiles)
                    printL1(pos);
                else
                    cout << string(CW, ' ');
                cout << "|";
            }
        } else {
            int cIdx = (row - 1) * 2;
            int posL = getPos(row, 0);
            if (posL < totalTiles)
                printL1(posL);
            else
                cout << string(CW, ' ');
            cout << "|" << interior[cIdx] << "|";
            int posR = getPos(row, S - 1);
            if (posR < totalTiles)
                printL1(posR);
            else
                cout << string(CW, ' ');
            cout << "|";
        }
        cout << "\n";

        // Line 2 (status / tokens)
        cout << "|";
        if (row == 0 || row == S - 1) {
            for (int col = 0; col < S; ++col) {
                int pos = getPos(row, col);
                if (pos < totalTiles)
                    printL2(pos);
                else
                    cout << string(CW, ' ');
                cout << "|";
            }
        } else {
            int cIdx = (row - 1) * 2 + 1;
            int posL = getPos(row, 0);
            if (posL < totalTiles)
                printL2(posL);
            else
                cout << string(CW, ' ');
            cout << "|" << interior[cIdx] << "|";
            int posR = getPos(row, S - 1);
            if (posR < totalTiles)
                printL2(posR);
            else
                cout << string(CW, ' ');
            cout << "|";
        }
        cout << "\n";
    }
    printFullSep();

    // ── Player info ─────────────────────────────────────────────────────────────
    cout << "\nPEMAIN: ";
    for (const auto* p : players) {
        if (p->getStatus() != PlayerStatus::BANKRUPT) {
            cout << "P" << (p->getId() + 1) << "=" << p->getUsername() << "(M" << p->getMoney()
                 << ",pos" << p->getPosition() << ")  ";
        }
    }
    cout << "\n\n";
}

void GameView::printPropertyDeed(const PropertyTile& property) {
    cout << "========================================\n";
    cout << "AKTA: " << property.getName() << " [" << property.getCode() << "]\n";
    cout << "Tipe: ";
    switch (property.getType()) {
    case PropertyType::STREET:
        cout << "Lahan";
        break;
    case PropertyType::RAILROAD:
        cout << "Stasiun";
        break;
    case PropertyType::UTILITY:
        cout << "Utility";
        break;
    }
    cout << "\n";
    cout << "Harga beli  : M" << property.getPrice() << "\n";
    cout << "Nilai gadai : M" << property.getMortgageValue() << "\n";
    cout << "Status      : ";
    if (property.isBankOwned())
        cout << "Milik Bank";
    else if (property.isMortgaged())
        cout << "DIGADAI oleh " << property.getOwner()->getUsername();
    else
        cout << "Dimiliki oleh " << property.getOwner()->getUsername();
    cout << "\n";

    const auto* street = dynamic_cast<const StreetTile*>(&property);
    if (street) {
        cout << "Color group : " << colorGroupToString(street->getColorGroup()) << "\n";
        cout << "Monopoli    : " << (street->isMonopolyOwned() ? "Ya" : "Tidak") << "\n";
        int lvl = street->getPropertyLevel();
        cout << "Bangunan    : ";
        if (lvl == 0)
            cout << "Kosong";
        else if (lvl == 5)
            cout << "Hotel";
        else
            cout << lvl << " rumah";
        cout << "\n";
        cout << "Harga rumah : M" << street->getHousePrice() << "\n";
        cout << "Harga hotel : M" << street->getHotelPrice() << "\n";
        if (street->hasFestival()) {
            cout << "Festival    : aktif x" << street->getFestivalMultiplier() << ", sisa "
                 << street->getFestivalDur() << " giliran\n";
        }
        cout << "--- Tabel Sewa ---\n";
        const string labels[] = {"Kosong", "1 Rumah", "2 Rumah", "3 Rumah", "4 Rumah", "Hotel"};
        const auto& rt = property.getRentTable();
        for (int i = 0; i < static_cast<int>(rt.size()) && i < 6; ++i) {
            cout << "  " << labels[i] << ": M" << rt[i] << "\n";
        }
    } else {
        cout << "--- Tabel Sewa ---\n";
        const auto& rt = property.getRentTable();
        for (int i = 0; i < static_cast<int>(rt.size()); ++i) {
            cout << "  Level " << i << ": M" << rt[i] << "\n";
        }
    }
    cout << "========================================\n";
}

void GameView::printPlayerProperties(const Player& player) {
    const auto& props = player.getProperties();
    if (props.empty()) {
        cout << player.getUsername() << " tidak memiliki properti.\n";
        return;
    }
    cout << "=== Properti milik " << player.getUsername() << " ===\n";
    for (const PropertyTile* p : props) {
        cout << "  [" << p->getCode() << "] " << p->getName();
        if (p->isMortgaged())
            cout << " [GADAI]";
        const auto* s = dynamic_cast<const StreetTile*>(p);
        if (s && s->getPropertyLevel() > 0) {
            int lvl = s->getPropertyLevel();
            cout << " [" << (lvl == 5 ? "Hotel" : to_string(lvl) + " Rumah") << "]";
        }
        if (s && s->hasFestival()) {
            cout << " [Festival x" << s->getFestivalMultiplier() << "]";
        }
        cout << " - Harga M" << p->getPrice() << "\n";
    }
    cout << "Total      : " << props.size() << " properti\n";
    cout << "Kekayaan   : M" << player.getTotalWealth() << "\n";
}

void GameView::renderAuctionPanel(int currentBid, const string& highBidder,
                                  const PropertyTile& tile) const {
    cout << "=== LELANG: " << tile.getName() << " [" << tile.getCode() << "] ===\n";
    cout << "Penawaran tertinggi: M" << currentBid << " oleh " << highBidder << "\n";
    cout << "Perintah: BID <jumlah> | PASS\n";
}

string GameView::getCommandInput(const Player& activePlayer) const {
    cout << "\n[" << activePlayer.getUsername() << " | M" << activePlayer.getMoney() << "] > ";
    string line;
    getline(cin, line);
    return line;
}

void GameView::printTransactionLogs(const vector<string>& logs) const {
    if (logs.empty()) {
        cout << "(Log kosong)\n";
        return;
    }
    cout << "=== Log Transaksi ===\n";
    for (const string& entry : logs) {
        cout << entry << "\n";
    }
    cout << "=====================\n";
}

void GameView::showEndGameScreen(const vector<string>& winners,
                                 const vector<string>& finalRankings) const {
    cout << "\n============================================\n";
    cout << "             PERMAINAN SELESAI!             \n";
    cout << "============================================\n";
    cout << "Pemenang: ";
    for (const string& w : winners)
        cout << w << "  ";
    cout << "\n\nPeringkat Akhir:\n";
    for (const string& r : finalRankings)
        cout << "  " << r << "\n";
    cout << "============================================\n";
}

// ── Prompts (moved from Game.cpp) ─────────────────────────────────────────────

bool GameView::promptBuyProperty(Player& player, PropertyTile& tile) {
    cout << "\nKamu mendarat di " << tile.getName() << " (" << tile.getCode() << ")!\n";
    printPropertyDeed(tile);
    cout << "Uang kamu saat ini: M" << player.getMoney() << "\n";
    cout << "Apakah kamu ingin membeli properti ini seharga M" << tile.getPrice() << "? (y/n): ";
    char c;
    if (player.getIsComputer()) {
        bool y = player.getMoney() > tile.getPrice() * 1.5 && (rand() % 2 == 0);
        c = y ? 'y' : 'n';
        cout << c << "\n";
    } else {
        string s = readLine();
        c = s.empty() ? 'n' : s[0];
    }
    return (c == 'y' || c == 'Y');
}

PropertyTile* GameView::promptSelectOpponentProperty(Player& player, const vector<Player*>& players,
                                                     const Board& board) {
    (void)board;
    vector<pair<Player*, PropertyTile*>> opts;
    for (auto* p : players) {
        if (p == &player || p->getStatus() == PlayerStatus::BANKRUPT)
            continue;
        for (PropertyTile* prop : p->getProperties()) {
            opts.push_back({p, prop});
        }
    }
    if (opts.empty()) {
        cout << "Tidak ada properti lawan yang dapat dipilih.\n";
        return nullptr;
    }
    cout << "=== Pilih Properti Lawan ===\n";
    for (int i = 0; i < static_cast<int>(opts.size()); i++) {
        auto* s = dynamic_cast<StreetTile*>(opts[i].second);
        int lvl = s ? s->getPropertyLevel() : 0;
        string lvlStr = "Tanpa bangunan";
        if (s) {
            lvlStr = (lvl == 5) ? "Hotel" : to_string(lvl) + " rumah";
        }
        cout << (i + 1) << ". " << opts[i].second->getName() << " (" << opts[i].second->getCode()
             << ") milik " << opts[i].first->getUsername() << " - " << lvlStr << "\n";
    }
    cout << "0. Batal\nPilih: ";
    int c = 0;
    if (player.getIsComputer()) {
        c = rand() % (opts.size() + 1);
        cout << c << "\n";
    } else {
        c = readInt();
    }
    if (c < 1 || c > static_cast<int>(opts.size()))
        return nullptr;
    return opts[c - 1].second;
}

Player* GameView::promptSelectTarget(Player& player, const vector<Player*>& players,
                                     const Board& board) {
    const int boardSize = board.getTotalTiles();
    const int myPos = player.getPosition();
    vector<Player*> targets;
    for (auto* p : players) {
        if (p == &player || p->getStatus() == PlayerStatus::BANKRUPT)
            continue;
        int dist = (p->getPosition() - myPos + boardSize) % boardSize;
        if (dist == 0)
            continue;
        targets.push_back(p);
    }
    if (targets.empty()) {
        cout << "Tidak ada pemain lawan yang berada di depan posisi kamu.\n";
        return nullptr;
    }
    cout << "=== Pilih Target Pemain (di depan posisi kamu) ===\n";
    for (int i = 0; i < static_cast<int>(targets.size()); i++) {
        cout << (i + 1) << ". " << targets[i]->getUsername() << " (posisi "
             << targets[i]->getPosition() << ")\n";
    }
    cout << "0. Batal\nPilih: ";
    int c = 0;
    if (player.getIsComputer()) {
        c = rand() % (targets.size() + 1);
        cout << c << "\n";
    } else {
        c = readInt();
    }
    if (c < 1 || c > static_cast<int>(targets.size()))
        return nullptr;
    return targets[c - 1];
}

int GameView::promptTaxChoice(Player& player, int flat, int pct) {
    cout << "\nKamu mendarat di Pajak Penghasilan (PPH)!\n";
    cout << "Pilih opsi pembayaran pajak:\n";
    cout << "1. Bayar flat M" << flat << "\n";
    cout << "2. Bayar " << pct << "% dari total kekayaan\n";
    cout << "Pilihan (1/2): ";
    int choice = 1;
    if (player.getIsComputer()) {
        choice = (rand() % 2) + 1;
        cout << choice << "\n";
    } else {
        choice = readInt();
    }
    if (choice == 2) {
        int totalWealth = player.getTotalWealth();
        int percentageAmount = totalWealth * pct / 100;
        cout << "Total kekayaan kamu:\n";
        cout << "  Uang tunai          : M" << player.getMoney() << "\n";
        cout << "  Harga beli properti : M" << player.getPropertyValue() << "\n";
        cout << "  Total               : M" << totalWealth << "\n";
        cout << "Pajak " << pct << "%             : M" << percentageAmount << "\n";
    }
    return (choice == 2) ? 2 : 1;
}

int GameView::promptTileIndex(Player& player, const Board& board) {
    (void)player;
    cout << "=== Pilih Petak Tujuan (TeleportCard) ===\n";
    for (int i = 0; i < board.getTotalTiles(); i++) {
        Tile* t = board.getTileAt(i);
        if (t)
            cout << i << ". " << t->getCode() << " - " << t->getName() << "\n";
    }
    cout << "Masukkan nomor petak (0-" << (board.getTotalTiles() - 1) << "): ";
    int idx = 0;
    if (player.getIsComputer()) {
        idx = rand() % board.getTotalTiles();
        cout << idx << "\n";
    } else {
        idx = readInt();
    }
    if (idx < 0 || idx >= board.getTotalTiles())
        idx = 0;
    return idx;
}

void GameView::promptFestivalSelection(Player& player) {
    const auto& props = player.getProperties();
    if (props.empty()) {
        cout << "Kamu tidak memiliki properti yang dapat dipilih.\n";
        return;
    }
    cout << "\nKamu mendarat di petak Festival!\n";
    cout << "Daftar properti milikmu:\n";
    for (int i = 0; i < static_cast<int>(props.size()); i++) {
        auto* p = props[i];
        string festInfo = "";
        if (p->hasFestival()) {
            festInfo = " [Festival aktif x" + to_string(p->getFestivalMultiplier()) + ", sisa " +
                       to_string(p->getFestivalDur()) + " giliran]";
        }
        cout << (i + 1) << ". " << p->getName() << " (" << p->getCode() << ")" << festInfo << "\n";
    }
    cout << "Masukkan nomor properti (0 untuk skip): ";
    int choice = 0;
    if (player.getIsComputer()) {
        choice = rand() % (props.size() + 1);
        cout << choice << "\n";
    } else {
        choice = readInt();
    }
    if (choice < 1 || choice > static_cast<int>(props.size())) {
        cout << "Efek festival tidak diterapkan.\n";
        return;
    }
    PropertyTile* selected = props[choice - 1];
    int oldMult = selected->getFestivalMultiplier();
    selected->activateFestival();
    int newMult = selected->getFestivalMultiplier();

    if (newMult > oldMult) {
        cout << "Efek festival aktif! Sewa " << selected->getName() << ": x" << oldMult << " -> x"
             << newMult << ". Durasi: 3 giliran.\n";
    } else {
        cout << "Efek sudah maksimum (harga sewa sudah digandakan tiga kali). "
                "Durasi di-reset menjadi 3 giliran.\n";
    }
}

pair<bool, int> GameView::promptAuctionBid(Player& player, int currentBid,
                                           const PropertyTile& tile) {
    while (true) {
        cout << "Giliran: " << player.getUsername() << "\n";
        cout << "Aksi (PASS / BID <jumlah>) > ";
        string line;
        if (player.getIsComputer()) {
            if (currentBid < tile.getPrice() * 1.5 && player.getMoney() > currentBid + 500 &&
                rand() % 2 == 0) {
                line = "BID " + to_string(currentBid + 10);
            } else {
                line = "PASS";
            }
            cout << line << "\n";
        } else {
            getline(cin, line);
        }
        istringstream iss(line);
        string cmd;
        iss >> cmd;
        transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
        if (cmd == "BID") {
            int amount = 0;
            if (iss >> amount && amount > currentBid && amount <= player.getMoney()) {
                return {true, amount};
            }
            if (amount <= currentBid)
                cout << "Penawaran harus lebih tinggi dari M" << currentBid << ".\n";
            else
                cout << "Uang kamu tidak cukup (M" << player.getMoney() << ").\n";
            // Re-prompt on invalid bid
            continue;
        }
        if (cmd == "PASS") {
            return {false, 0};
        }
        cout << "Perintah tidak valid. Gunakan PASS atau BID <jumlah>.\n";
    }
}

bool GameView::runLiquidationPanel(Player& debtor, int amountNeeded, Player* creditor,
                                   const vector<Player*>& players, const Board& board,
                                   int currentTurn, Logger& logger) {
    (void)players;
    cout << "\nKamu tidak dapat membayar M" << amountNeeded << "!\n";
    cout << "Uang kamu       : M" << debtor.getMoney() << "\n";
    cout << "Total kewajiban : M" << amountNeeded << "\n";
    cout << "Kekurangan      : M" << (amountNeeded - debtor.getMoney()) << "\n";
    cout << "\nDana likuidasi dapat menutup kewajiban. Kamu wajib melikuidasi aset.\n";

    while (debtor.getMoney() < amountNeeded) {
        cout << "\n=== Panel Likuidasi ===\n";
        cout << "Uang kamu saat ini: M" << debtor.getMoney() << "  |  Kewajiban: M" << amountNeeded
             << "\n";

        vector<PropertyTile*> sellable, mortgageable;
        for (auto* prop : debtor.getProperties()) {
            if (prop->isMortgaged())
                continue;
            sellable.push_back(prop);
        }
        for (auto* prop : debtor.getProperties()) {
            if (prop->getStatus() == PropertyStatus::OWNED)
                mortgageable.push_back(prop);
        }

        if (sellable.empty() && mortgageable.empty()) {
            cout << "Tidak ada aset yang dapat dilikuidasi.\n";
            return false;
        }

        auto liquidateColorGroupBuildings = [&](StreetTile* selectedStreet) {
            if (!selectedStreet)
                return;

            ColorGroup cg = selectedStreet->getColorGroup();
            vector<StreetTile*> groupStreets;
            bool hasBuildings = false;
            for (int i = 0; i < board.getTotalTiles(); ++i) {
                auto* street = dynamic_cast<StreetTile*>(board.getTileAt(i));
                if (street && street->getOwner() == &debtor && street->getColorGroup() == cg) {
                    groupStreets.push_back(street);
                    if (street->getPropertyLevel() > 0) {
                        hasBuildings = true;
                    }
                }
            }
            if (!hasBuildings)
                return;

            cout << "Masih ada bangunan di color group [" << colorGroupToString(cg)
                 << "]. Seluruh bangunan pada color group tersebut dijual terlebih dahulu.\n";
            for (auto* street : groupStreets) {
                int lvl = street->getPropertyLevel();
                if (lvl == 0)
                    continue;

                int refund = 0;
                if (lvl == 5) {
                    refund = (street->getHotelPrice() + 4 * street->getHousePrice()) / 2;
                } else {
                    refund = lvl * street->getHousePrice() / 2;
                }
                street->setPropertyLevel(0);
                debtor += refund;
                cout << "Bangunan " << street->getName() << " terjual. Kamu menerima M" << refund
                     << ".\n";
                logger.logEvent(LogLevel::INFO, currentTurn, debtor.getUsername(),
                                "JUAL_BANGUNAN",
                                "Likuidasi bangunan " + street->getName() + " M" +
                                    to_string(refund));
            }
        };

        cout << "[Jual ke Bank]\n";
        for (int i = 0; i < static_cast<int>(sellable.size()); i++) {
            auto* s = dynamic_cast<StreetTile*>(sellable[i]);
            int sellVal = sellable[i]->getPrice();
            if (s) {
                int lvl = s->getPropertyLevel();
                if (lvl == 5) {
                    sellVal += (s->getHotelPrice() + 4 * s->getHousePrice()) / 2;
                } else {
                    sellVal += lvl * s->getHousePrice() / 2;
                }
            }
            cout << (i + 1) << ". " << sellable[i]->getName() << " (" << sellable[i]->getCode()
                 << ")  Harga Jual: M" << sellVal << "\n";
        }
        cout << "[Gadaikan]\n";
        for (int i = 0; i < static_cast<int>(mortgageable.size()); i++) {
            cout << (static_cast<int>(sellable.size()) + i + 1) << ". "
                 << mortgageable[i]->getName() << " (" << mortgageable[i]->getCode()
                 << ")  Nilai Gadai: M" << mortgageable[i]->getMortgageValue() << "\n";
        }
        cout << "Pilih aksi (0 jika sudah cukup): ";

        int choice = 0;
    if (debtor.getIsComputer()) {
            choice = (rand() % (sellable.size() + mortgageable.size())) + 1;
            cout << choice << "\n";
        } else {
            choice = readInt();
        }

        if (choice == 0) {
            cout << "Kamu belum boleh keluar dari panel likuidasi sebelum kewajiban terpenuhi.\n";
            continue;
        }

        if (choice >= 1 && choice <= static_cast<int>(sellable.size())) {
            auto* prop = sellable[choice - 1];
            auto* s = dynamic_cast<StreetTile*>(prop);
            int sellVal = prop->getPrice();
            if (s) {
                int lvl = s->getPropertyLevel();
                if (lvl == 5) {
                    sellVal += (s->getHotelPrice() + 4 * s->getHousePrice()) / 2;
                } else {
                    sellVal += lvl * s->getHousePrice() / 2;
                }
                s->setPropertyLevel(0);
            }
            debtor.removeProperty(prop);
            prop->setFestivalState(1, 0);
            prop->releaseToBank();
            debtor += sellVal;
            cout << prop->getName() << " terjual ke Bank. Kamu menerima M" << sellVal << ".\n";
            logger.logEvent(LogLevel::INFO, currentTurn, debtor.getUsername(), "LIKUIDASI_JUAL",
                            prop->getCode() + " M" + to_string(sellVal));
        } else {
            int mIdx = choice - static_cast<int>(sellable.size()) - 1;
            if (mIdx >= 0 && mIdx < static_cast<int>(mortgageable.size())) {
                auto* prop = mortgageable[mIdx];
                auto* street = dynamic_cast<StreetTile*>(prop);
                liquidateColorGroupBuildings(street);
                prop->mortgage();
                debtor += prop->getMortgageValue();
                cout << prop->getName() << " digadaikan. Kamu menerima M"
                     << prop->getMortgageValue() << ".\n";
                logger.logEvent(LogLevel::INFO, currentTurn, debtor.getUsername(), "GADAI",
                                "Likuidasi gadai " + prop->getCode() + " M" +
                                    to_string(prop->getMortgageValue()));
            }
        }
    }

    // Execute payment
    debtor -= amountNeeded;
    if (creditor) {
        *creditor += amountNeeded;
        cout << "Kewajiban M" << amountNeeded << " terpenuhi. Membayar ke "
             << creditor->getUsername() << "...\n";
        cout << "Uang " << debtor.getUsername() << ": M" << debtor.getMoney() << "\n";
        cout << "Uang " << creditor->getUsername() << ": M" << creditor->getMoney() << "\n";
    } else {
        cout << "Kewajiban M" << amountNeeded << " terpenuhi. Membayar ke Bank...\n";
    }
    return true;
}
