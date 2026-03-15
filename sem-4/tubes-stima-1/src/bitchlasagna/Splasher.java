package bitchlasagna;

import battlecode.common.*;

public class Splasher extends Robot {

    /**
     * score a single splash center based on nearby enemies and paint
     */
    private static int scoreSplashTarget(RobotController rc, MapLocation center,
            RobotInfo[] robots, MapInfo[] tiles) {
        int score = 0;

        for (RobotInfo unit : robots) {
            if (unit.getTeam() == rc.getTeam()) continue;
            int dist = center.distanceSquaredTo(unit.getLocation());

            if (dist <= Constants.SPLASHER_ATTACK_RADIUS) {
                if (unit.getType().isTowerType()) {
                    score += Constants.SPLASHER_TOWER_SCORE;
                } else if (dist <= Constants.SPLASHER_INNER_RADIUS) {
                    score += Constants.SPLASHER_ENEMY_ROBOT_SCORE;
                }
            }
        }

        for (MapInfo tileInfo : tiles) {
            int dist = center.distanceSquaredTo(tileInfo.getMapLocation());

            if (dist <= Constants.SPLASHER_ATTACK_RADIUS) {
                if (tileInfo.getPaint().isEnemy() && dist <= Constants.SPLASHER_INNER_RADIUS) {
                    score += Constants.SPLASHER_ENEMY_PAINT_SCORE;
                } else if (tileInfo.getPaint() == PaintType.EMPTY) {
                    score += Constants.SPLASHER_EMPTY_PAINT_SCORE;
                }
            }
        }

        return score;
    }

    /**
     * find best splash target and attack, prioritizing towers > robots > paint
     */
    public static void splashAttack(RobotController rc) throws GameActionException {
        MapInfo[] centerCandidates = rc.senseNearbyMapInfos(Constants.SPLASHER_ATTACK_RADIUS);
        RobotInfo[] allNearbyRobots = rc.senseNearbyRobots(Constants.GLOBAL_SENSE_RADIUS);
        MapInfo[] allMapInfos = rc.senseNearbyMapInfos(Constants.GLOBAL_SENSE_RADIUS);

        MapLocation bestTarget = null;
        int maxScore = 0;

        for (MapInfo centerInfo : centerCandidates) {
            MapLocation center = centerInfo.getMapLocation();
            if (!rc.canAttack(center)) continue;

            int score = scoreSplashTarget(rc, center, allNearbyRobots, allMapInfos);
            if (score > maxScore) {
                maxScore = score;
                bestTarget = center;
            }
        }

        if (bestTarget != null) {
            rc.attack(bestTarget);
        }
    }

    /**
     * move toward nearest enemy, prioritizing towers. explore if none visible
     */
    public static void moveToEnemy(RobotController rc) throws GameActionException {
        MapLocation targetLoc = Sensing.findEnemyTarget(rc);

        if (targetLoc != null) {
            Direction dir = Pathfinding.pathfind(rc, targetLoc);
            if (dir != null && rc.canMove(dir)) {
                rc.move(dir);
            }
        } else {
            Direction dir = Pathfinding.exploreUnpainted(rc);
            if (dir != null && rc.canMove(dir)) {
                rc.move(dir);
            }
        }
    }
}