package weball;

import battlecode.common.*;

public class Mopper extends Robot {

    /**
     * prioritize Mop > Swing > Transfer
     */
    public static void mopperAction(RobotController rc) throws GameActionException {
        MapInfo[] nearbyTiles = rc.senseNearbyMapInfos(rc.getType().actionRadiusSquared);
        MapLocation bestMopTarget = null;
        int maxEnemyPaintCount = 0;

        for (MapInfo tile : nearbyTiles) {
            MapLocation tileLoc = tile.getMapLocation();
            if (rc.canAttack(tileLoc) && tile.getPaint().isEnemy()) {
                int enemyPaintScore = Constants.MOPPER_ENEMY_PAINT_SCORE; 
                
                if (enemyPaintScore > maxEnemyPaintCount) {
                    maxEnemyPaintCount = enemyPaintScore;
                    bestMopTarget = tileLoc;
                }
            }
        }

        // Mop enemy tiles first
        if (bestMopTarget != null) {
            rc.attack(bestMopTarget);
            return;
        }

        // if no enemy tiles, Swing
        RobotInfo[] nearbyRobots = rc.senseNearbyRobots(Constants.MOPPER_SENSE_RADIUS);
        RobotInfo enemyTarget = null;
        int maxPaintEnemy = -1;
        RobotInfo healTarget = null;
        int minPaintAlly = Integer.MAX_VALUE;

        for (RobotInfo robot : nearbyRobots) {
            if (robot.getTeam() != rc.getTeam()) {
                if (robot.getPaintAmount() > maxPaintEnemy) {
                    maxPaintEnemy = robot.getPaintAmount();
                    enemyTarget = robot;
                }
            } else {
                if (robot.getType() != UnitType.MOPPER && robot.getPaintAmount() < minPaintAlly) {
                    minPaintAlly = robot.getPaintAmount();
                    healTarget = robot;
                }
            }
        }

        if (enemyTarget != null && rc.canAttack(enemyTarget.getLocation())) {
            rc.attack(enemyTarget.getLocation());
            return;
        }

        // Transfer if no enemy tiles
        if (healTarget != null
                && rc.getPaint() > Constants.MOPPER_MIN_PAINT_TO_TRANSFER
                && rc.canTransferPaint(healTarget.getLocation(), Constants.MOPPER_TRANSFER_AMOUNT)) {
            rc.transferPaint(healTarget.getLocation(), Constants.MOPPER_TRANSFER_AMOUNT);
        }
    }

    /**
     * move toward enemy territory
     */
    public static void moveToPerimeter(RobotController rc) throws GameActionException {
        MapLocation myLoc = rc.getLocation();
        Direction bestDir = null;
        int maxScore = Integer.MIN_VALUE;
        MapInfo[] allMapInfos = rc.senseNearbyMapInfos(Constants.GLOBAL_SENSE_RADIUS);

        for (Direction dir : Constants.DIRECTIONS) {
            if (!rc.canMove(dir)) continue;

            MapLocation nextLoc = myLoc.add(dir);
            MapInfo nextLocInfo = rc.senseMapInfo(nextLoc);
            int currScore = 0;

            // avoid team tiles
            if (nextLocInfo.getPaint().isAlly()) {
                currScore -= (Constants.MOPPER_ALLY_PAINT_SCORE / 2);
            }

            // focus on enemy tiles
            for (MapInfo adjInfo : allMapInfos) {
                if (nextLoc.distanceSquaredTo(adjInfo.getMapLocation()) <= Constants.MOPPER_SENSE_RADIUS) {
                    if (adjInfo.getPaint().isEnemy()) {
                        currScore += (Constants.MOPPER_BORDER_TILE_SCORE * 4); 
                    } 
                    else if (adjInfo.getPaint() == PaintType.EMPTY) {
                        currScore += (Constants.MOPPER_BORDER_TILE_SCORE / 2);
                    }
                }
            }

            if (currScore > maxScore) {
                maxScore = currScore;
                bestDir = dir;
            }
        }

        if (bestDir != null) {
            rc.move(bestDir);
        } else {
            // Fallback: find enemies
            Direction exploreDir = Pathfinding.exploreUnpainted(rc);
            if (exploreDir != null && rc.canMove(exploreDir)) {
                rc.move(exploreDir);
            }
        }
    }
}