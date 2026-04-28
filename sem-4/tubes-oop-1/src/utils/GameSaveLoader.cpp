#include "GameSaveLoader.hpp"
#include "BoardFactory.hpp"
#include "ConfigManager.hpp"
#include "DemolitionCard.hpp"
#include "DiscountCard.hpp"
#include "Exceptions.hpp"
#include "Game.hpp"
#include "LassoCard.hpp"
#include "MoveCard.hpp"
#include "Player.hpp"
#include "PropertyTile.hpp"
#include "ShieldCard.hpp"
#include "SkillCard.hpp"
#include "StreetTile.hpp"
#include "TeleportCard.hpp"
#include "TurnManager.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

// ── Format helpers ────────────────────────────────────────────────────────────

static string cardToToken(const SkillCard* card) {
    if (auto* m = dynamic_cast<const MoveCard*>(card))
        return "MoveCard " + to_string(m->getSteps());
    if (auto* d = dynamic_cast<const DiscountCard*>(card))
        return "DiscountCard " + to_string(d->getDiscountPercentage()) + " " +
               to_string(d->getDuration());
    if (dynamic_cast<const ShieldCard*>(card))
        return "ShieldCard";
    if (dynamic_cast<const TeleportCard*>(card))
        return "TeleportCard";
    if (dynamic_cast<const LassoCard*>(card))
        return "LassoCard";
    if (dynamic_cast<const DemolitionCard*>(card))
        return "DemolitionCard";
    return "UnknownCard";
}

static unique_ptr<SkillCard> extractCardFromStream(istringstream& ss, const string& type) {
    if (type == "MoveCard") {
        int steps = 1;
        ss >> steps;
        return make_unique<MoveCard>("MoveCard", "Maju " + to_string(steps) + " petak.", steps);
    }
    if (type == "DiscountCard") {
        int pct = 10, dur = 1;
        ss >> pct >> dur;
        return make_unique<DiscountCard>("DiscountCard", "Diskon " + to_string(pct) + "%.", pct,
                                         dur);
    }
    if (type == "ShieldCard") {
        return make_unique<ShieldCard>("ShieldCard", "Kebal tagihan.", 1);
    }
    if (type == "TeleportCard")
        return make_unique<TeleportCard>("TeleportCard", "Pindah ke petak manapun.");
    if (type == "LassoCard")
        return make_unique<LassoCard>("LassoCard", "Tarik lawan ke posisimu.");
    if (type == "DemolitionCard")
        return make_unique<DemolitionCard>("DemolitionCard", "Hancurkan bangunan lawan.");
    return nullptr;
}

static string buildingLevelToToken(int level) {
    return (level == 5) ? "H" : to_string(level);
}

static int buildingLevelFromToken(const string& token) {
    if (token == "H" || token == "h")
        return 5;
    try {
        return stoi(token);
    } catch (...) {
        return 0;
    }
}

// ── Save ──────────────────────────────────────────────────────────────────────

static const string SAVE_CONFIG_PREFIX = "CONFIG_DIR ";

GameSaveLoader::GameSaveLoader() {}

GameSaveLoader::~GameSaveLoader() {}

void GameSaveLoader::save(const Game& game, const string& filename) const {
    ofstream out(filename);
    if (!out)
        throw FileException(filename, "simpan");

    out << SAVE_CONFIG_PREFIX << game.currentConfigDir << "\n";

    const TurnManager& tm = game.turnManager;
    // <TURN_SAAT_INI> <MAX_TURN> <JUMLAH_PEMAIN>
    out << tm.getCurrentTurn() << " " << tm.getMaxTurn() << " " << game.players.size() << "\n";

    // <STATE_PEMAIN_1> to <STATE_PEMAIN_N>
    // Format: <USERNAME> <UANG> <POSISI_PETAK> <STATUS> <JUMLAH_KARTU_TANGAN> <JENIS_KARTU_1> <NILAI> ...
    for (const auto& p : game.players) {
        string statusStr;
        switch (p->getStatus()) {
        case PlayerStatus::ACTIVE:
            statusStr = "ACTIVE";
            break;
        case PlayerStatus::BANKRUPT:
            statusStr = "BANKRUPT";
            break;
        case PlayerStatus::JAILED:
            statusStr = "JAILED";
            break;
        }

        string posCode = "GO";
        Tile* t = game.board->getTileAt(p->getPosition());
        if (t)
            posCode = t->getCode();

        out << p->getUsername() << " " << p->getMoney() << " " << posCode << " " << statusStr
            << " ";

        const auto& hand = p->getHand();
        out << hand.size();
        for (const SkillCard* card : hand) {
            out << " " << cardToToken(card);
        }
        out << " " << (p->hasJailFreeCard() ? 1 : 0);
        out << "\n";
    }

    // <URUTAN_GILIRAN_1> ... <URUTAN_GILIRAN_N> <GILIRAN_AKTIF_SAAT_INI>
    const vector<int>& order = tm.getTurnOrder();
    for (int idx : order) {
        out << game.players[idx]->getUsername() << " ";
    }
    int activeIdx = tm.getActivePlayerIndex();
    if (activeIdx >= 0 && activeIdx < static_cast<int>(game.players.size())) {
        out << game.players[activeIdx]->getUsername() << "\n";
    } else {
        out << "UNKNOWN\n";
    }

    // <STATE_PROPERTI>
    vector<PropertyTile*> properties;
    int total = game.board->getTotalTiles();
    for (int i = 0; i < total; ++i) {
        if (auto* prop = dynamic_cast<PropertyTile*>(game.board->getTileAt(i))) {
            properties.push_back(prop);
        }
    }

    out << properties.size() << " ";
    for (size_t i = 0; i < properties.size(); ++i) {
        auto* prop = properties[i];

        string typeStr;
        switch (prop->getType()) {
        case PropertyType::STREET:
            typeStr = "street";
            break;
        case PropertyType::RAILROAD:
            typeStr = "railroad";
            break;
        case PropertyType::UTILITY:
            typeStr = "utility";
            break;
        }

        string owner = prop->getOwner() ? prop->getOwner()->getUsername() : "BANK";

        string propStatus;
        switch (prop->getStatus()) {
        case PropertyStatus::BANK:
            propStatus = "BANK";
            break;
        case PropertyStatus::OWNED:
            propStatus = "OWNED";
            break;
        case PropertyStatus::MORTGAGED:
            propStatus = "MORTGAGED";
            break;
        }

        int level = 0;
        if (auto* s = dynamic_cast<StreetTile*>(prop)) {
            level = s->getPropertyLevel();
        }

        out << prop->getCode() << " " << typeStr << " " << owner << " " << propStatus << " "
            << prop->getFestivalMultiplier() << " " << prop->getFestivalDur() << " "
            << buildingLevelToToken(level);

        if (i < properties.size() - 1)
            out << "\n";
    }
    out << "\n";

    // <STATE_DECK>
    const auto& allCards = game.allSkillCards;
    vector<SkillCard*> deckCards;
    // Find unassigned cards
    for (auto& c : allCards) {
        bool inHand = false;
        for (const auto& player : game.players) {
            if (player->hasCard(c.get())) {
                inHand = true;
                break;
            }
        }
        if (!inHand)
            deckCards.push_back(c.get());
    }

    out << deckCards.size();
    for (const SkillCard* c : deckCards) {
        out << " " << cardToToken(c);
    }
    out << "\n";

    // <STATE_LOG>
    const vector<string>& logs = game.logger.getSaveLogs();
    out << logs.size() << "\n";
    for (const string& entry : logs) {
        out << entry << "\n";
    }
}

// ── Load ──────────────────────────────────────────────────────────────────────

void GameSaveLoader::load(Game& game, const string& filename) const {
    ifstream in(filename);
    if (!in)
        throw FileException(filename, "tidak_ditemukan");

    string line;
    if (!getline(in, line))
        throw FileException(filename, "muat");

    string configDir = "config";
    if (line.rfind(SAVE_CONFIG_PREFIX, 0) == 0) {
        string savedDir = line.substr(SAVE_CONFIG_PREFIX.size());
        if (!savedDir.empty()) {
            configDir = savedDir;
        }
        if (!getline(in, line))
            throw FileException(filename, "muat");
    }

    ConfigManager cfg(configDir);
    cfg.loadAll();
    game.config = cfg.getConfig();
    game.currentConfigDir = configDir;
    auto [newBoard, newChanceCards, newCommunityCards, newAllSkillCards, newChanceDeck,
          newCommunityDeck, newSkillDeck] = BoardFactory::build(cfg);
    game.board = move(newBoard);
    game.chanceCards = move(newChanceCards);
    game.communityCards = move(newCommunityCards);
    game.allSkillCards = move(newAllSkillCards);
    game.chanceDeck = move(newChanceDeck);
    game.communityDeck = move(newCommunityDeck);
    game.skillDeck = move(newSkillDeck);

    // <TURN_SAAT_INI> <MAX_TURN> <JUMLAH_PEMAIN>
    istringstream ss(line);
    int turnSaatIni, maxTurn, jumlahPemain;
    if (!(ss >> turnSaatIni >> maxTurn >> jumlahPemain)) {
        throw FileException(filename, "format corrupt");
    }

    game.turnManager = TurnManager(maxTurn);
    game.players.clear();
    game.allSkillCards.clear();

    // <STATE_PEMAIN>
    for (int i = 0; i < jumlahPemain; ++i) {
        if (!getline(in, line))
            throw FileException(filename, "kurang_baris_pemain");
        istringstream ps(line);
        string username, posCode, statusStr;
        int uang, jumlahKartu;
        ps >> username >> uang >> posCode >> statusStr >> jumlahKartu;

        auto player = make_unique<Player>(i, username);
        *player += uang;

        // Find posCode tile index (or 0 if not found)
        int posIdx = 0;
        int totalTiles = game.board->getTotalTiles();
        for (int t = 0; t < totalTiles; ++t) {
            if (game.board->getTileAt(t)->getCode() == posCode) {
                posIdx = t;
                break;
            }
        }
        player->setPosition(posIdx);

        if (statusStr == "BANKRUPT")
            player->setStatus(PlayerStatus::BANKRUPT);
        else if (statusStr == "JAILED") {
            player->setStatus(PlayerStatus::JAILED);
            player->setJailTurns(1); // default
        } else {
            player->setStatus(PlayerStatus::ACTIVE);
        }

        for (int k = 0; k < jumlahKartu; ++k) {
            string type;
            if (ps >> type) {
                auto card = extractCardFromStream(ps, type);
                if (card) {
                    SkillCard* raw = card.get();
                    game.allSkillCards.push_back(move(card));
                    try {
                        player->addCard(raw);
                    } catch (...) {
                    }
                }
            }
        }
        int hasJailFreeCard = 0;
        if (ps >> hasJailFreeCard) {
            player->setJailFreeCard(hasJailFreeCard != 0);
        }
        game.players.push_back(move(player));
    }

    // <URUTAN_GILIRAN> <GILIRAN_AKTIF>
    if (!getline(in, line))
        throw FileException(filename, "kurang_baris_urutan");
    istringstream ts(line);
    vector<int> order;
    vector<string> usernames;
    string token;
    while (ts >> token)
        usernames.push_back(token);

    if (usernames.empty())
        throw FileException(filename, "urutan_kosong");
    string activeUsername = usernames.back();
    usernames.pop_back(); // Remove active username from end of list

    int activeIndex = 0;
    for (size_t i = 0; i < usernames.size(); ++i) {
        for (size_t p = 0; p < game.players.size(); ++p) {
            if (game.players[p]->getUsername() == usernames[i]) {
                order.push_back(static_cast<int>(p));
                if (usernames[i] == activeUsername) {
                    activeIndex = static_cast<int>(i);
                }
                break;
            }
        }
    }

    // <STATE_PROPERTI>
    if (!getline(in, line))
        throw FileException(filename, "kurang_baris_properti");
    istringstream propsStream(line);
    int numProps;
    if (!(propsStream >> numProps))
        numProps = 0;

    string code, typeStr, ownerUsername, propStatus, buildingToken;
    int fMult, fDur;

    // The rest of the first property line
    if (propsStream >> code >> typeStr >> ownerUsername >> propStatus >> fMult >> fDur >>
        buildingToken) {
        auto* tile = dynamic_cast<PropertyTile*>(game.board->getTileByCode(code));
        if (tile) {
            Player* owner = nullptr;
            for (auto& p : game.players)
                if (p->getUsername() == ownerUsername) {
                    owner = p.get();
                    break;
                }

            if (propStatus == "BANK")
                tile->setStatus(PropertyStatus::BANK);
            else if (propStatus == "OWNED") {
                tile->setStatus(PropertyStatus::OWNED);
                tile->setOwner(owner);
            } else if (propStatus == "MORTGAGED") {
                tile->setStatus(PropertyStatus::MORTGAGED);
                tile->setOwner(owner);
            }

            tile->setFestivalState(fMult, fDur);
            if (auto* s = dynamic_cast<StreetTile*>(tile))
                s->setPropertyLevel(buildingLevelFromToken(buildingToken));

            if (owner && propStatus != "BANK")
                owner->addProperty(tile);
        }
    }

    for (int i = 1; i < numProps; ++i) {
        if (!getline(in, line))
            break;
        istringstream ps(line);
        if (ps >> code >> typeStr >> ownerUsername >> propStatus >> fMult >> fDur >>
            buildingToken) {
            auto* tile = dynamic_cast<PropertyTile*>(game.board->getTileByCode(code));
            if (!tile)
                continue;

            Player* owner = nullptr;
            for (auto& p : game.players)
                if (p->getUsername() == ownerUsername) {
                    owner = p.get();
                    break;
                }

            if (propStatus == "BANK")
                tile->setStatus(PropertyStatus::BANK);
            else if (propStatus == "OWNED") {
                tile->setStatus(PropertyStatus::OWNED);
                tile->setOwner(owner);
            } else if (propStatus == "MORTGAGED") {
                tile->setStatus(PropertyStatus::MORTGAGED);
                tile->setOwner(owner);
            }

            tile->setFestivalState(fMult, fDur);
            if (auto* s = dynamic_cast<StreetTile*>(tile))
                s->setPropertyLevel(buildingLevelFromToken(buildingToken));

            if (owner && propStatus != "BANK")
                owner->addProperty(tile);
        }
    }

    // Update monopolies after properties are loaded
    game.updateMonopolyStatus();
    game.turnManager.loadOrder(order, activeIndex, turnSaatIni);

    // <STATE_DECK>
    if (getline(in, line)) {
        istringstream ds(line);
        int numDeckCards;
        if (ds >> numDeckCards) {
            vector<SkillCard*> deckRaw;
            string cardType;
            for (int i = 0; i < numDeckCards; ++i) {
                if (ds >> cardType) {
                    auto card = extractCardFromStream(ds, cardType);
                    if (card) {
                        deckRaw.push_back(card.get());
                        game.allSkillCards.push_back(move(card));
                    }
                }
            }
            game.skillDeck = make_unique<CardDeck<SkillCard>>(deckRaw);
        }
    }

    // <STATE_LOG>
    game.logger.clear();
    if (getline(in, line)) {
        istringstream lsc(line);
        int numLogs;
        if (lsc >> numLogs) {
            for (int i = 0; i < numLogs; ++i) {
                if (getline(in, line)) {
                    istringstream ls(line);
                    int turn;
                    string user, action;
                    if (ls >> turn >> user >> action) {
                        // remaining is details, read rest of the string
                        string details;
                        getline(ls >> ws, details);
                        game.logger.logEvent(LogLevel::INFO, turn, user, action, details);
                    }
                }
            }
        }
    }
}
