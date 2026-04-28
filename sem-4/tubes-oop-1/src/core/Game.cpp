#include "Game.hpp"
#include "Board.hpp"
#include "BoardFactory.hpp"
#include "ChanceCard.hpp"
#include "ColorGroup.hpp"
#include "CommandHandler.hpp"
#include "CommunityChestCard.hpp"
#include "ConfigManager.hpp"
#include "Dice.hpp"
#include "Exceptions.hpp"
#include "FinanceManager.hpp"
#include "GameSaveLoader.hpp"
#include "IUserInteraction.hpp"
#include "Player.hpp"
#include "PropertyManager.hpp"
#include "PropertyTile.hpp"
#include "RailroadTile.hpp"
#include "SkillCard.hpp"
#include "StreetTile.hpp"
#include "Tile.hpp"
#include "UtilityTile.hpp"
#include <algorithm>
#include <sstream>
#include <string>

using namespace std;

Game::Game()
    : currentConfigDir("config"), state(GameState::MENU), lastDiceTotal(0), turnManager(0),
      ui(nullptr) {
    dice = make_unique<Dice>();
    board = make_unique<Board>();
}

Game::~Game() = default;

void Game::setUserInteraction(IUserInteraction* userInteraction) {
    ui = userInteraction;
}

// ── Helpers ──────────────────────────────────────────────────────────────────

vector<Player*> Game::getActivePlayers() const {
    vector<Player*> active;
    for (auto& p : players) {
        if (p->getStatus() != PlayerStatus::BANKRUPT)
            active.push_back(p.get());
    }
    return active;
}

void Game::updateMonopolyStatus() {
    if (!board)
        return;
    for (int i = 0; i < board->getTotalTiles(); i++) {
        auto* prop = dynamic_cast<PropertyTile*>(board->getTileAt(i));
        if (!prop)
            continue;
        auto* street = dynamic_cast<StreetTile*>(prop);
        if (!street)
            continue;

        ColorGroup cg = street->getColorGroup();
        Player* owner = street->getOwner();
        if (!owner) {
            street->setMonopolyOwned(false);
            continue;
        }

        bool allOwn = true;
        for (int j = 0; j < board->getTotalTiles(); j++) {
            auto* other = dynamic_cast<StreetTile*>(board->getTileAt(j));
            if (other && other->getColorGroup() == cg && other->getOwner() != owner) {
                allOwn = false;
                break;
            }
        }
        street->setMonopolyOwned(allOwn);
    }
}

void Game::distributeSkillCards() {
    for (auto& p : players) {
        for (int i = 0; i < 2 && skillDeck && !skillDeck->isEmpty(); ++i) {
            SkillCard* card = skillDeck->draw();
            if (card) {
                try {
                    p->addCard(card);
                } catch (...) {
                    skillDeck->discard(card);
                }
            }
        }
    }
}

// ── Game lifecycle ────────────────────────────────────────────────────────────

void Game::resetGameData() {
    players.clear();
    chanceCards.clear();
    communityCards.clear();
    allSkillCards.clear();
    chanceDeck.reset();
    communityDeck.reset();
    skillDeck.reset();
    board = make_unique<Board>();
    dice = make_unique<Dice>();
    logger.clear();
    currentConfigDir = "config";
    turnManager = TurnManager(0);
    lastDiceTotal = 0;
    state = GameState::MENU;
}

void Game::createGame(const string& configDir) {
    resetGameData();
    ConfigManager cfg(configDir);
    cfg.loadAll();
    config = cfg.getConfig();
    currentConfigDir = configDir;
    turnManager = TurnManager(config.maxTurn);

    // Delegate board construction to BoardFactory
    auto [newBoard, newChanceCards, newCommunityCards, newAllSkillCards, newChanceDeck,
          newCommunityDeck, newSkillDeck] = BoardFactory::build(cfg);
    board = move(newBoard);
    chanceCards = move(newChanceCards);
    communityCards = move(newCommunityCards);
    allSkillCards = move(newAllSkillCards);
    chanceDeck = move(newChanceDeck);
    communityDeck = move(newCommunityDeck);
    skillDeck = move(newSkillDeck);

    auto setups = ui->promptPlayerSetup();
    for (int i = 0; i < static_cast<int>(setups.size()); ++i) {
        players.push_back(make_unique<Player>(i, setups[i].first));
        players.back()->setIsComputer(setups[i].second);
        *players.back() += config.startingBalance;
    }

    turnManager.initOrder(static_cast<int>(players.size()));
    state = GameState::PLAYING;
}

void Game::loadGame(const string& filename) {
    resetGameData();
    GameSaveLoader loader;
    loader.load(*this, filename);
    state = GameState::PLAYING;
    if (!players.empty()) {
        logger.logEvent(LogLevel::INFO, getCurrentTurn(), getActivePlayer().getUsername(), "MUAT",
                        "Muat dari " + filename + " (" + currentConfigDir + ")");
    }
}

void Game::saveGame(const string& filename) const {
    GameSaveLoader loader;
    loader.save(*this, filename);
}

void Game::runCycle() {
    if (state != GameState::PLAYING)
        return;
    Player& active = getActivePlayer();
    runTurn(active);
    if (active.getStatus() == PlayerStatus::BANKRUPT) {
        turnManager.removePlayer(active.getId());
    }
    auto activePlayers = getActivePlayers();
    if (activePlayers.size() <= 1) {
        state = GameState::GAMEOVER;
        return;
    }
    if (active.getStatus() != PlayerStatus::BANKRUPT) {
        turnManager.advance(activePlayers);
    }
    if (turnManager.hasReachedMaxTurn()) {
        state = GameState::GAMEOVER;
    }
}

bool Game::checkWin() const {
    return state == GameState::GAMEOVER;
}

// ── Per-turn logic ────────────────────────────────────────────────────────────

void Game::runTurn(Player& player) {
    player.resetTurnFlags();
    dice->resetDoublesCount();

    // Distribute 1 skill card at start of turn
    if (skillDeck && !skillDeck->isEmpty()) {
        SkillCard* newCard = skillDeck->draw();
        if (newCard)
            handleCardDrop(player, newCard);
    }

    // Tick turn-based card effects
    player.tickDiscount();
    player.tickShield();

    // Tick festival on player's own properties
    for (auto* prop : player.getProperties()) {
        prop->tickFestival();
    }

    // Jail handling replaces normal turn
    if (player.isInJail()) {
        bool extraTurnFromJail = handleJailTurn(player);
        if (!extraTurnFromJail)
            return;
        // Exited jail via double — grant one extra command loop
        dice->resetDoublesCount();
        player.setRolledDice(false);
        while (!player.getHasRolledDice()) {
            if (player.isInJail())
                return;
            if (!ui)
                return;
            string input;
            if (player.getIsComputer()) {
                if (!player.getHasUsedSkillThisTurn() && !player.getHand().empty() &&
                    rand() % 100 < 20) {
                    input = "GUNAKAN_KEMAMPUAN";
                } else if (rand() % 100 < 10) {
                    input = "TEBUS";
                } else if (rand() % 100 < 10) {
                    input = "BANGUN";
                } else {
                    input = "LEMPAR_DADU";
                }
                ui->printMessage("\n[" + player.getUsername() + " (COM) | M" +
                                 to_string(player.getMoney()) + "] > " + input + "\n");
            } else {
                input = ui->getCommandInput(player);
            }
            handleCommand(input, player);
        }
        return;
    }

    // Command loop — may cycle multiple times when player gets double
    bool extraTurn = false;
    do {
        extraTurn = false;
        player.setRolledDice(false);

        while (!player.getHasRolledDice()) {
            if (player.isInJail())
                return;
            if (!ui)
                return;
            string input;
            if (player.getIsComputer()) {
                if (!player.getHasUsedSkillThisTurn() && !player.getHand().empty() &&
                    rand() % 100 < 20) {
                    input = "GUNAKAN_KEMAMPUAN";
                } else if (rand() % 100 < 10) {
                    input = "TEBUS";
                } else if (rand() % 100 < 10) {
                    input = "BANGUN";
                } else {
                    input = "LEMPAR_DADU";
                }
                ui->printMessage("\n[" + player.getUsername() + " (COM) | M" +
                                 to_string(player.getMoney()) + "] > " + input + "\n");
            } else {
                input = ui->getCommandInput(player);
            }
            handleCommand(input, player);
        }

        if (player.isInJail())
            return;

        if (dice->isDouble()) {
            extraTurn = true;
            ui->printMessage("Double! " + player.getUsername() + " mendapat giliran tambahan.\n");
        }
    } while (extraTurn);
}

bool Game::handleJailTurn(Player& player) {
    ui->printMessage("\n=== " + player.getUsername() + " berada di Penjara ===\n");
    int jailTurns = player.getJailTurns();
    ui->printMessage("Giliran penjara ke-" + to_string(jailTurns + 1) + "\n");

    // Turn 4 (jailTurns == 3): forced payment
    if (jailTurns >= 3) {
        ui->printMessage("Giliran ke-4 di penjara. Wajib membayar denda M" +
                         to_string(config.jailFine) + ".\n");
        chargeVoluntary(player, config.jailFine);
        if (player.getStatus() != PlayerStatus::BANKRUPT) {
            player.releaseFromJail();
            ui->printMessage(player.getUsername() + " keluar dari penjara.\n");
            // Take regular roll
            lastDiceTotal = dice->rollRandom();
            auto dv = dice->getDiceValues();
            ui->printMessage("Dadu: " + to_string(dv.first) + " + " + to_string(dv.second) + " = " +
                             to_string(lastDiceTotal) + "\n");
            logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "DADU",
                            "Lempar: " + to_string(dv.first) + "+" + to_string(dv.second) + "=" +
                                to_string(lastDiceTotal) + " (keluar penjara, bayar denda)");
            movePlayerBy(player, lastDiceTotal);
        }
        return false;
    }

    ui->printMessage("Opsi keluar dari penjara:\n");
    ui->printMessage("1. BAYAR - bayar denda M" + to_string(config.jailFine) + "\n");
    ui->printMessage("2. LEMPAR - coba lempar double\n");
    if (player.hasJailFreeCard()) {
        ui->printMessage("3. KARTU - gunakan kartu Bebas dari Penjara\n");
    }
    ui->printMessage("Pilih (BAYAR/LEMPAR" + string(player.hasJailFreeCard() ? "/KARTU" : "") +
                     "): ");

    string choice;
    if (player.getIsComputer()) {
        int r = rand() % 100;
        if (r < 10 && player.getMoney() >= config.jailFine)
            choice = "BAYAR";
        else if (player.hasJailFreeCard() && r < 40)
            choice = "KARTU";
        else
            choice = "LEMPAR";
        ui->printMessage(choice + "\n");
    } else {
        choice = ui->readLine();
    }
    transform(choice.begin(), choice.end(), choice.begin(), ::toupper);

    if (choice == "KARTU" || choice == "3") {
        if (!player.hasJailFreeCard()) {
            ui->printMessage("Kamu tidak memiliki kartu Bebas dari Penjara.\n");
            player.incrementJailTurns();
            return false;
        }
        player.useJailFreeCard();
        player.releaseFromJail();
        ui->printMessage(player.getUsername() +
                         " menggunakan kartu Bebas dari Penjara dan keluar dari penjara!\n");
        logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "PENJARA",
                        "Keluar penjara via kartu Bebas dari Penjara");
        lastDiceTotal = dice->rollRandom();
        auto dv = dice->getDiceValues();
        ui->printMessage("Dadu: " + to_string(dv.first) + " + " + to_string(dv.second) + " = " +
                         to_string(lastDiceTotal) + "\n");
        movePlayerBy(player, lastDiceTotal);
        return false;
    } else if (choice == "BAYAR" || choice == "1") {
        chargeVoluntary(player, config.jailFine);
        if (player.getStatus() != PlayerStatus::BANKRUPT) {
            player.releaseFromJail();
            ui->printMessage(player.getUsername() + " membayar M" + to_string(config.jailFine) +
                             " dan keluar dari penjara.\n");
            logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "PENJARA",
                            "Bayar denda M" + to_string(config.jailFine) + " dan keluar penjara");
            lastDiceTotal = dice->rollRandom();
            auto dv = dice->getDiceValues();
            ui->printMessage("Dadu: " + to_string(dv.first) + " + " + to_string(dv.second) + " = " +
                             to_string(lastDiceTotal) + "\n");
            movePlayerBy(player, lastDiceTotal);
        }
        return false;
    } else {
        // Try to roll double
        dice->resetDoublesCount();
        lastDiceTotal = dice->rollRandom();
        auto dv = dice->getDiceValues();
        ui->printMessage("Dadu: " + to_string(dv.first) + " + " + to_string(dv.second) + " = " +
                         to_string(lastDiceTotal) + "\n");
        logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "DADU",
                        "Lempar di penjara: " + to_string(dv.first) + "+" + to_string(dv.second) +
                            "=" + to_string(lastDiceTotal));

        if (dice->isDouble()) {
            ui->printMessage("Double! " + player.getUsername() + " keluar dari penjara.\n");
            player.releaseFromJail();
            logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "PENJARA",
                            "Keluar penjara via double");
            movePlayerBy(player, lastDiceTotal);
            return true;
        } else {
            ui->printMessage("Tidak double. " + player.getUsername() + " tetap di penjara.\n");
            player.incrementJailTurns();
            return false;
        }
    }
}

void Game::handleCardDrop(Player& player, SkillCard* newCard) {
    if (!newCard)
        return;
    ui->printMessage("\nKamu mendapatkan 1 kartu acak baru!\n");
    ui->printMessage("Kartu yang didapat: " + newCard->getName() + " - " +
                     newCard->getDescription() + "\n");

    try {
        player.addCard(newCard);
        ui->printMessage("Kartu ditambahkan ke tangan. Tangan: " +
                         to_string(player.getHand().size()) + " kartu.\n");
    } catch (const CardLimitException&) {
        ui->printMessage("PERINGATAN: Kamu sudah memiliki 3 kartu di tangan (Maksimal 3)!\n");
        ui->printMessage("Kamu diwajibkan membuang 1 kartu.\n");

        const auto& hand = player.getHand();
        ui->printMessage("Daftar Kartu Kemampuan Anda:\n");
        for (int i = 0; i < static_cast<int>(hand.size()); i++) {
            ui->printMessage(to_string(i + 1) + ". " + hand[i]->getName() + " - " +
                             hand[i]->getDescription() + "\n");
        }
        ui->printMessage(to_string(hand.size() + 1) + ". " + newCard->getName() + " - " +
                         newCard->getDescription() + " [BARU]\n");
        ui->printMessage("Pilih nomor kartu yang ingin dibuang (1-" + to_string(hand.size() + 1) +
                         "): ");

        int dropChoice;
        if (player.getIsComputer()) {
            dropChoice = (rand() % (hand.size() + 1)) + 1;
            ui->printMessage(to_string(dropChoice) + "\n");
        } else {
            dropChoice = ui->readInt();
        }

        if (dropChoice >= 1 && dropChoice <= static_cast<int>(hand.size())) {
            SkillCard* toDiscard = hand[dropChoice - 1];
            player.removeCard(toDiscard);
            skillDeck->discard(toDiscard);
            ui->printMessage(toDiscard->getName() + " telah dibuang.\n");
            player.addCard(newCard);
        } else {
            // Discard the new card (also covers out-of-range)
            skillDeck->discard(newCard);
            ui->printMessage(newCard->getName() + " (baru) telah dibuang.\n");
        }
        ui->printMessage("Sekarang kamu memiliki " + to_string(player.getHand().size()) +
                         " kartu di tangan.\n");
    }
}

// ── Command dispatcher (delegates to CommandHandler) ──────────────────────────

void Game::handleCommand(const string& input, Player& player) {
    commandHandler.dispatch(
        input, player, *this, propertyManager, *board, *dice, skillDeck.get(), ui, logger,
        lastDiceTotal, [this](const string& filename) { saveGame(filename); }, players);
}

// ── IGameContext: state queries ───────────────────────────────────────────────

Player& Game::getActivePlayer() {
    int idx = turnManager.getActivePlayerIndex();
    return *players[idx];
}

const vector<Player*> Game::getAllActivePlayers() const {
    return getActivePlayers();
}

Board& Game::getBoard() {
    return *board;
}

const GameConfig& Game::getConfig() const {
    return config;
}

int Game::getCurrentTurn() const {
    return turnManager.getCurrentTurn();
}

Logger& Game::getLogger() {
    return logger;
}

int Game::getLastDiceTotal() const {
    return lastDiceTotal;
}

void Game::printMessage(const string& message) {
    if (ui)
        ui->printMessage(message);
}

// ── Public accessors ──────────────────────────────────────────────────────────

const vector<unique_ptr<Player>>& Game::getPlayers() const {
    return players;
}

const TurnManager& Game::getTurnManager() const {
    return turnManager;
}

const vector<unique_ptr<SkillCard>>& Game::getAllSkillCards() const {
    return allSkillCards;
}

GameState Game::getState() const {
    return state;
}

// ── IGameContext: player movement ─────────────────────────────────────────────

void Game::movePlayerBy(Player& player, int steps) {
    int oldPos = player.getPosition();
    int newPos = board->getNewPosition(oldPos, steps);
    bool passGo = steps > 0 && (oldPos + steps >= board->getTotalTiles());
    player.setPosition(newPos);

    Tile* t = board->getTileAt(newPos);
    string tileName = t ? t->getName() : "?";
    ui->printMessage("Bidak mendarat di: " + tileName + ".\n");
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "GERAK",
                    "Posisi " + to_string(oldPos) + " -> " + to_string(newPos) + " (" + tileName +
                        ")");

    if (passGo)
        grantSalary(player);
    if (t)
        t->landedOn(*this);
}

void Game::movePlayerTo(Player& player, int tileIndex) {
    int oldPos = player.getPosition();
    bool passGo = (tileIndex < oldPos);
    player.setPosition(tileIndex);

    Tile* t = board->getTileAt(tileIndex);
    string tileName = t ? t->getName() : "?";
    ui->printMessage("Bidak dipindahkan ke: " + tileName + ".\n");
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "GERAK",
                    "Posisi " + to_string(oldPos) + " -> " + to_string(tileIndex) + " (" +
                        tileName + ")");

    if (passGo)
        grantSalary(player);
    if (t)
        t->landedOn(*this);
}

void Game::repositionPlayer(Player& player, int tileIndex) {
    int oldPos = player.getPosition();
    player.setPosition(tileIndex);

    Tile* t = board->getTileAt(tileIndex);
    string tileName = t ? t->getName() : "?";
    ui->printMessage(player.getUsername() + " dipindahkan ke " + tileName + " (tanpa efek).\n");
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "REPOSITION",
                    "Posisi " + to_string(oldPos) + " -> " + to_string(tileIndex) + " (" +
                        tileName + ")");
}

void Game::movePlayerToNearest(Player& player, const string& tileType) {
    int current = player.getPosition();
    int total = board->getTotalTiles();
    for (int i = 1; i < total; ++i) {
        int pos = (current + i) % total;
        Tile* t = board->getTileAt(pos);
        if (!t)
            continue;
        auto* prop = dynamic_cast<PropertyTile*>(t);
        if (prop && prop->getType() == PropertyType::RAILROAD) {
            movePlayerTo(player, pos);
            return;
        }
    }
}

void Game::sendPlayerToJail(Player& player) {
    player.goToJail();
    player.setPosition(board->getJailPosition());
    dice->resetDoublesCount();
    ui->printMessage(player.getUsername() + " masuk penjara!\n");
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "PENJARA",
                    "Masuk penjara di petak " + to_string(board->getJailPosition()));
}

// ── IGameContext: financial operations (delegated to FinanceManager) ─────────

void Game::grantSalary(Player& player) {
    financeManager.grantSalary(player, config, getCurrentTurn(), logger, ui);
}

void Game::transferMoney(Player& payer, Player& collector, int amount) {
    financeManager.transferMoney(payer, collector, amount, getCurrentTurn(), logger, *this, ui);
}

void Game::chargeToBank(Player& player, int amount) {
    financeManager.chargeToBank(player, amount, getCurrentTurn(), logger, *this, ui);
}

void Game::chargeVoluntary(Player& player, int amount) {
    financeManager.chargeVoluntary(player, amount, getCurrentTurn(), logger, *this, ui);
}

void Game::collectFromAll(Player& collector, int amountPerPlayer) {
    financeManager.collectFromAll(collector, amountPerPlayer, getActivePlayers(), getCurrentTurn(),
                                  logger, *this, ui);
}

void Game::payToAll(Player& payer, int amountPerPlayer) {
    financeManager.payToAll(payer, amountPerPlayer, getActivePlayers(), getCurrentTurn(), logger,
                            *this, ui);
}

// ── IGameContext: property operations ────────────────────────────────────────

void Game::grantProperty(Player& player, PropertyTile& tile) {
    // Remove from previous owner if any
    if (tile.getOwner() && tile.getOwner() != &player) {
        tile.getOwner()->removeProperty(&tile);
    }
    tile.setOwner(&player);
    player.addProperty(&tile);
    updateMonopolyStatus();
    ui->printMessage(tile.getName() + " kini menjadi milik " + player.getUsername() + ".\n");
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "PROPERTI",
                    "Dapat " + tile.getName() + " (" + tile.getCode() + ")");
}

void Game::initiateAuction(PropertyTile& tile) {
    const auto& turnOrder = turnManager.getTurnOrder();
    if (turnOrder.empty())
        return;

    int triggerId = getActivePlayer().getId();
    int triggerPos = 0;
    for (int i = 0; i < static_cast<int>(turnOrder.size()); ++i) {
        if (turnOrder[i] == triggerId) {
            triggerPos = i;
            break;
        }
    }

    vector<Player*> bidderOrder;
    for (int offset = 1; offset <= static_cast<int>(turnOrder.size()); ++offset) {
        int playerId = turnOrder[(triggerPos + offset) % static_cast<int>(turnOrder.size())];
        Player* candidate = players[playerId].get();
        if (candidate->getStatus() != PlayerStatus::BANKRUPT) {
            bidderOrder.push_back(candidate);
        }
    }

    auctionManager.runAuction(tile, bidderOrder, *this);
}

void Game::destroyPropertyToBank(PropertyTile& tile) {
    Player* owner = tile.getOwner();
    if (!owner)
        return;

    if (auto* street = dynamic_cast<StreetTile*>(&tile)) {
        street->setPropertyLevel(0);
    }
    tile.setFestivalState(1, 0);
    owner->removeProperty(&tile);
    tile.releaseToBank();
    updateMonopolyStatus();
    ui->printMessage(tile.getName() + " dihancurkan dan dikembalikan ke Bank.\n");
    logger.logEvent(LogLevel::INFO, getCurrentTurn(), getActivePlayer().getUsername(), "DEMOLISH",
                    tile.getCode() + " dikembalikan ke Bank");
}

void Game::triggerBankruptcy(Player& debtor, Player* creditor, int amount) {
    GameState prevState = state;
    state = GameState::LIQUIDATION;
    bankruptcyHandler.handle(debtor, creditor, amount, *this);
    state = prevState;
}

void Game::refreshMonopolyStatus() {
    updateMonopolyStatus();
}

// ── IGameContext: UI-mediated interactions ────────────────────────────────────

bool Game::promptBuyProperty(Player& player, PropertyTile& tile) {
    if (!ui)
        return false;
    return ui->promptBuyProperty(player, tile);
}

PropertyTile* Game::promptSelectOpponentProperty(Player& player) {
    if (!ui)
        return nullptr;
    return ui->promptSelectOpponentProperty(player, getActivePlayers(), *board);
}

Player* Game::promptSelectTarget(Player& player) {
    if (!ui)
        return nullptr;
    return ui->promptSelectTarget(player, getActivePlayers(), *board);
}

int Game::promptTaxChoice(Player& player, int flat, int pct) {
    if (!ui)
        return 1;
    return ui->promptTaxChoice(player, flat, pct);
}

int Game::promptTileIndex(Player& player) {
    if (!ui)
        return 0;
    return ui->promptTileIndex(player, *board);
}

void Game::promptFestivalSelection(Player& player) {
    if (!ui)
        return;
    vector<pair<PropertyTile*, pair<int, int>>> before;
    for (auto* prop : player.getProperties()) {
        before.push_back({prop, {prop->getFestivalMultiplier(), prop->getFestivalDur()}});
    }
    ui->promptFestivalSelection(player);
    for (const auto& entry : before) {
        PropertyTile* prop = entry.first;
        int oldMult = entry.second.first;
        int oldDur = entry.second.second;
        int newMult = prop->getFestivalMultiplier();
        int newDur = prop->getFestivalDur();
        if (oldMult != newMult || oldDur != newDur) {
            logger.logEvent(LogLevel::INFO, getCurrentTurn(), player.getUsername(), "FESTIVAL",
                            prop->getCode() + ": x" + to_string(oldMult) + " -> x" +
                                to_string(newMult) + ", durasi " + to_string(oldDur) + " -> " +
                                to_string(newDur));
            break;
        }
    }
}

pair<bool, int> Game::promptAuctionBid(Player& player, int currentBid, const PropertyTile& tile) {
    if (!ui)
        return {false, 0};
    return ui->promptAuctionBid(player, currentBid, tile);
}

bool Game::runLiquidationPanel(Player& debtor, int amountNeeded, Player* creditor) {
    if (!ui)
        return false;
    bool paid = ui->runLiquidationPanel(debtor, amountNeeded, creditor, getActivePlayers(), *board,
                                        getCurrentTurn(), logger);
    updateMonopolyStatus();
    return paid;
}
