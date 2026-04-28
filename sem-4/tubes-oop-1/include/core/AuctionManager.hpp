#pragma once
#include <vector>

class Player;
class PropertyTile;
class IGameContext;

/**
 * @brief Handles the full auction flow when a property goes up for bid.
 * Single Responsibility: auction mechanics only.
 */
class AuctionManager {
public:
    AuctionManager();
    ~AuctionManager();

    /**
     * @brief Run an auction for the given tile.
     *
     * @param tile The property to be auctioned.
     * @param bidderOrder All eligible bidders, already ordered from the first bidder.
     * @param ctx Game context for transferring money and assigning ownership.
     */
    void runAuction(PropertyTile& tile,
                    const std::vector<Player*>& bidderOrder,
                    IGameContext& ctx);
};
