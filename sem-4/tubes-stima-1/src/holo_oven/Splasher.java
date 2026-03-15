package holo_oven;

import battlecode.common.*;

public class Splasher extends Robot {

    /**
     * find best splash target and attack, prioritizing towers > robots > paint
     */
    public static void splashAttack(RobotController rc) throws GameActionException {
        MapInfo[] centerCandidates = rc.senseNearbyMapInfos(Constants.SPLASHER_ATTACK_RADIUS);
        MapLocation bestTarget = null;
        int maxScore = 0;
        RobotInfo[] allNearbyRobots = rc.senseNearbyRobots(Constants.GLOBAL_SENSE_RADIUS);
        MapInfo[] allMapInfos = rc.senseNearbyMapInfos(Constants.GLOBAL_SENSE_RADIUS);

        for (MapInfo centerInfo : centerCandidates) {
            MapLocation center = centerInfo.getMapLocation();

            if (!rc.canAttack(center)) continue;

            int currScore = 0;

            for (RobotInfo unit : allNearbyRobots) {
                if (unit.getTeam() == rc.getTeam()) continue;
                int dist = center.distanceSquaredTo(unit.getLocation());

                if (dist <= Constants.SPLASHER_ATTACK_RADIUS) {
                    if (unit.getType().isTowerType()) {
                        currScore += Constants.SPLASHER_TOWER_SCORE;
                    } else if (dist <= Constants.SPLASHER_INNER_RADIUS) {
                        currScore += Constants.SPLASHER_ENEMY_ROBOT_SCORE;
                    }
                }
            }

            for (MapInfo tileInfo : allMapInfos) {
                int dist = center.distanceSquaredTo(tileInfo.getMapLocation());

                if (dist <= Constants.SPLASHER_ATTACK_RADIUS) {
                    if (tileInfo.getPaint().isEnemy() && dist <= Constants.SPLASHER_INNER_RADIUS) {
                        currScore += Constants.SPLASHER_ENEMY_PAINT_SCORE;
                    } else if (tileInfo.getPaint() == PaintType.EMPTY) {
                        currScore += Constants.SPLASHER_EMPTY_PAINT_SCORE;
                    }
                }
            }

            if (currScore > maxScore) {
                maxScore = currScore;
                bestTarget = center;
            }
        }

        if (bestTarget != null) {
            rc.attack(bestTarget);
        }
    }

    /**
     * move toward nearest enemy, prioritizing towers
     */
    public static void moveToEnemy(RobotController rc) throws GameActionException {
        RobotInfo[] visibleEnemies = rc.senseNearbyRobots(Constants.GLOBAL_SENSE_RADIUS, rc.getTeam().opponent());
        MapLocation targetLoc = null;

        // prioritize the nearest enemy, with a score bonus for towers.
        int bestScore = Integer.MAX_VALUE;
        for (RobotInfo enemy : visibleEnemies) {
            int score = rc.getLocation().distanceSquaredTo(enemy.getLocation());
            if (enemy.getType().isTowerType()) {
                score -= 8;
            }
            if (score < bestScore) {
                bestScore = score;
                targetLoc = enemy.getLocation();
            }
        }

        if (targetLoc != null) {
            // pathfinding towards target
            Direction dir = Pathfinding.pathfindAttack(rc, targetLoc);
            if (dir != null && rc.canMove(dir)) {
                rc.move(dir);
            }
        } else {
            // no enemy visible, advance toward center to find fights faster
            Direction dir = Pathfinding.pathfindAttack(rc, Robot.getMapCenter(rc));
            if (dir != null && rc.canMove(dir)) {
                rc.move(dir);
            }
        }
    }
}