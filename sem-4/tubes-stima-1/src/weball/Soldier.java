package weball;

import battlecode.common.*;

public class Soldier extends Robot {

    public enum SoldierState {
        ADVANCE,
        ATTACK
    }

    /**
     * find the worth of tile to paint
     */
    public static boolean tryPaint(RobotController rc, MapInfo paintTile) throws GameActionException {
        MapLocation paintLocation = paintTile.getMapLocation();
        
        if (rc.canAttack(paintLocation)) {
            // prioritize enemy tile
            if (paintTile.getPaint().isEnemy()) {
                rc.attack(paintLocation);
                return true;
            }
            else if (paintTile.getPaint() == PaintType.EMPTY) {
                rc.attack(paintLocation);
                return true;
            }
        }
        return false;
    }

    public static void attackEnemyTower(RobotController rc, MapInfo[] nearbyTiles) throws GameActionException {
        for (MapInfo nearbyTile : nearbyTiles) {
            if (nearbyTile.hasRuin() && rc.canSenseRobotAtLocation(nearbyTile.getMapLocation())
                    && !rc.senseRobotAtLocation(nearbyTile.getMapLocation()).getTeam().equals(rc.getTeam())) {
                RobotPlayer.enemyTower = nearbyTile;
                if (rc.canAttack(nearbyTile.getMapLocation())) {
                    rc.attack(nearbyTile.getMapLocation());
                } else {
                    Direction dir = Pathfinding.pathfind(rc, nearbyTile.getMapLocation());
                    if (dir != null) {
                        rc.move(dir);
                    }
                }
                return;
            }
        }
        RobotPlayer.enemyTower = null;

        // if no tower, keep moving towards enemy tower
        RobotInfo enemyTowerBot = Sensing.towerInRange(rc, rc.getType().actionRadiusSquared, false);
        if (enemyTowerBot != null) {
            Direction dir = Pathfinding.pathfind(rc, enemyTowerBot.getLocation());
            if (dir != null) {
                rc.move(dir);
            }
        }

        if (RobotPlayer.enemyTile != null
                && rc.canSenseLocation(RobotPlayer.enemyTile.getMapLocation())
                && !rc.senseMapInfo(RobotPlayer.enemyTile.getMapLocation()).getPaint().isEnemy()) {
            RobotPlayer.enemyTile = null;
            RobotPlayer.soldierState = SoldierState.ADVANCE;
        }
    }

    public static void paintAroundSelf(RobotController rc) throws GameActionException {
        MapInfo[] nearby = rc.senseNearbyMapInfos(rc.getType().actionRadiusSquared);

        // prioritize enemy tiles
        for (MapInfo tile : nearby) {
            if (!rc.isActionReady()) return;
            if (tile.isPassable() && tile.getPaint().isEnemy() && rc.canAttack(tile.getMapLocation())) {
                rc.attack(tile.getMapLocation());
            }
        }

        // fallback: emppty tiles
        for (MapInfo tile : nearby) {
            if (!rc.isActionReady()) return;
            if (tile.isPassable() && tile.getPaint() == PaintType.EMPTY && rc.canAttack(tile.getMapLocation())) {
                rc.attack(tile.getMapLocation());
            }
        }
    }

    // find enemy > empty tiles
    public static boolean explore(RobotController rc) throws GameActionException {
        paintAroundSelf(rc);

        Direction dir = Pathfinding.exploreUnpainted(rc); 
        
        // fallback: find empty tiles
        if (dir == null) {
            dir = Pathfinding.exploreUnpainted(rc); 
        }

        if (dir != null && rc.canMove(dir)) {
            rc.move(dir);

            paintAroundSelf(rc);
            return true;
        }

        return false;
    }
}