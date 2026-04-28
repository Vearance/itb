#include "AuctionManager.hpp"
#include "IGameContext.hpp"
#include "Logger.hpp"
#include "Player.hpp"
#include "PropertyTile.hpp"
#include <string>
#include <vector>

using namespace std;

AuctionManager::AuctionManager() {}

AuctionManager::~AuctionManager() {}

void AuctionManager::runAuction(PropertyTile& tile, const vector<Player*>& bidderOrder,
                                IGameContext& ctx) {
    if (bidderOrder.empty())
        return;

    const int bidderCount = static_cast<int>(bidderOrder.size());
    int highBid = -1;
    Player* highBidder = nullptr;
    int consecutivePasses = 0;
    int currentIndex = 0;

    ctx.printMessage("\n=== LELANG: " + tile.getName() + " [" + tile.getCode() + "] ===\n");
    ctx.printMessage("Harga beli asli: M" + to_string(tile.getPrice()) + "\n");
    ctx.printMessage("Masukkan BID <jumlah> untuk menawar, atau PASS untuk melewati.\n\n");

    while (true) {
        Player* bidder = bidderOrder[currentIndex];
        const bool mustBid = (highBidder == nullptr && consecutivePasses == bidderCount - 1);
        string highName = highBidder ? highBidder->getUsername() : "Tidak ada";
        int displayBid = highBid < 0 ? 0 : highBid;
        ctx.printMessage("Penawaran tertinggi saat ini: M" + to_string(displayBid) + " oleh " +
                         highName + "\n");
        if (mustBid) {
            ctx.printMessage(bidder->getUsername() +
                             " wajib melakukan BID agar lelang memiliki minimal satu penawaran.\n");
        }

        pair<bool, int> bidResult = ctx.promptAuctionBid(*bidder, highBid, tile);
        bool didBid = bidResult.first;
        int amount = bidResult.second;

        if (didBid && amount > highBid) {
            highBid = amount;
            highBidder = bidder;
            consecutivePasses = 0;
            ctx.printMessage(bidder->getUsername() + " menawar M" + to_string(amount) + "!\n");
            ctx.getLogger().logEvent(LogLevel::INFO, ctx.getCurrentTurn(), bidder->getUsername(),
                                     "LELANG_BID", tile.getCode() + " M" + to_string(amount));
        } else if (mustBid) {
            if (bidder->getIsComputer()) {
                int forcedBid = (highBid < 0) ? 0 : highBid;
                highBid = forcedBid;
                highBidder = bidder;
                consecutivePasses = 0;
                ctx.printMessage(bidder->getUsername() + " terpaksa menawar minimum M" +
                                 to_string(forcedBid) + "!\n");
                ctx.getLogger().logEvent(LogLevel::INFO, ctx.getCurrentTurn(),
                                         bidder->getUsername(), "LELANG_BID",
                                         tile.getCode() + " M" + to_string(forcedBid));
            } else {
                ctx.printMessage("Belum ada penawaran. " + bidder->getUsername() +
                                 " harus memasukkan BID yang valid.\n");
                continue;
            }
        } else {
            ++consecutivePasses;
            ctx.printMessage(bidder->getUsername() + " melewati lelang.\n");
            ctx.getLogger().logEvent(LogLevel::INFO, ctx.getCurrentTurn(), bidder->getUsername(),
                                     "LELANG_PASS", tile.getCode());
            if (highBidder != nullptr && consecutivePasses >= bidderCount - 1) {
                break;
            }
        }

        currentIndex = (currentIndex + 1) % bidderCount;
    }

    ctx.printMessage("\n=== HASIL LELANG ===\n");
    if (highBidder && highBid >= 0) {
        ctx.printMessage(highBidder->getUsername() + " memenangkan lelang dengan penawaran M" +
                         to_string(highBid) + "!\n");
        ctx.getLogger().logEvent(LogLevel::INFO, ctx.getCurrentTurn(), highBidder->getUsername(),
                                 "LELANG_MENANG", tile.getCode() + " M" + to_string(highBid));
        ctx.chargeVoluntary(*highBidder, highBid);
        if (highBidder->getStatus() != PlayerStatus::BANKRUPT) {
            ctx.grantProperty(*highBidder, tile);
        }
    } else {
        ctx.printMessage("Tidak ada pemenang lelang. " + tile.getName() + " tetap milik Bank.\n");
    }
}
