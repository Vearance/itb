package holo_oven;

import battlecode.common.*;

public class Mopper extends Robot {

    /**
     * transfer paint to low allies, mop enemy robots
     */
    public static void mopperAction(RobotController rc) throws GameActionException {
        RobotInfo[] nearbyRobots = rc.senseNearbyRobots(Constants.MOPPER_SENSE_RADIUS);
        RobotInfo healTarget = null;
        RobotInfo enemyTarget = null;
        int minPaintAlly = Integer.MAX_VALUE;
        int maxPaintEnemy = -1;

        for (RobotInfo robot : nearbyRobots) {
            if (robot.getTeam() == rc.getTeam()) {
                if (robot.getType() != UnitType.MOPPER && robot.getPaintAmount() < minPaintAlly) {
                    minPaintAlly = robot.getPaintAmount();
                    healTarget = robot;
                }
            } else {
                if (robot.getPaintAmount() > maxPaintEnemy) {
                    maxPaintEnemy = robot.getPaintAmount();
                    enemyTarget = robot;
                }
            }
        }

        // transfer paint to lowest-paint ally
        if (healTarget != null
                && rc.getPaint() > Constants.MOPPER_MIN_PAINT_TO_TRANSFER
                && rc.canTransferPaint(healTarget.getLocation(), Constants.MOPPER_TRANSFER_AMOUNT)) {
            rc.transferPaint(healTarget.getLocation(), Constants.MOPPER_TRANSFER_AMOUNT);
            return;
        }

        // mop enemy robot
        if (enemyTarget != null && rc.canAttack(enemyTarget.getLocation())) {
            rc.attack(enemyTarget.getLocation());
        }
    }

    /**
     * move toward the border between ally and enemy/empty territory
     */
    public static void moveToPerimeter(RobotController rc) throws GameActionException {
        MapLocation myLoc = rc.getLocation();
        Direction bestDir = null;
        int maxScore = Integer.MIN_VALUE;
        RobotInfo[] allAllies = rc.senseNearbyRobots(Constants.GLOBAL_SENSE_RADIUS, rc.getTeam());
        MapInfo[] allMapInfos = rc.senseNearbyMapInfos(Constants.GLOBAL_SENSE_RADIUS);

        for (Direction dir : Constants.DIRECTIONS) {
            if (!rc.canMove(dir)) continue;

            MapLocation nextLoc = myLoc.add(dir);
            MapInfo nextLocInfo = rc.senseMapInfo(nextLoc);

            if (nextLocInfo.getPaint().isEnemy()) continue;

            int currScore = 0;

            if (nextLocInfo.getPaint().isAlly()) {
                currScore += Constants.MOPPER_ALLY_PAINT_SCORE;
            }

            for (RobotInfo ally : allAllies) {
                if (nextLoc.distanceSquaredTo(ally.getLocation()) <= Constants.MOPPER_SENSE_RADIUS) {
                    if (ally.getType() != UnitType.MOPPER
                            && ally.getPaintAmount() < Constants.MOPPER_LOW_ALLY_PAINT) {
                        currScore += Constants.MOPPER_NEAR_LOW_ALLY_SCORE;
                    }
                }
            }

            for (MapInfo adjInfo : allMapInfos) {
                if (nextLoc.distanceSquaredTo(adjInfo.getMapLocation()) <= Constants.MOPPER_SENSE_RADIUS) {
                    if (adjInfo.getPaint() == PaintType.EMPTY || adjInfo.getPaint().isEnemy()) {
                        currScore += Constants.MOPPER_BORDER_TILE_SCORE;
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
        }
    }
}