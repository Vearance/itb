package bitchlasagna;

import battlecode.common.*;

/**
 * Methods for general robot behavior.
 */
public class Robot {
    // low paint = penalty cooldown; tolerate 40% for further robots
    public static boolean hasLowPaint(RobotController rc, int percentage) {
        if (rc.getPaint() < (rc.getType().paintCapacity) * percentage / 100) {
            return true;
        } else {
            return false;
        }
    }

    public static boolean hasLowHealth(RobotController rc, int percentage) {
        if (rc.getHealth() < (rc.getType().health) * percentage / 100) {
            return true;
        } else {
            return false;
        }
    }

    public static void markTower(RobotController rc, UnitType towerType, MapLocation targetLoc) throws GameActionException {
        MapLocation shouldBeMarked = targetLoc.subtract(rc.getLocation().directionTo(targetLoc));
        if (rc.senseMapInfo(shouldBeMarked).getMark() == PaintType.EMPTY && rc.canMarkTowerPattern(towerType, targetLoc)) {
            rc.markTowerPattern(towerType, targetLoc);
        }
    }

    public static void completeTower(RobotController rc, MapLocation towerLocation) throws GameActionException {
        if (rc.canCompleteTowerPattern(UnitType.LEVEL_ONE_MONEY_TOWER, towerLocation)) {
            rc.completeTowerPattern(UnitType.LEVEL_ONE_MONEY_TOWER, towerLocation);
        }
        if (rc.canCompleteTowerPattern(UnitType.LEVEL_ONE_PAINT_TOWER, towerLocation)) {
            rc.completeTowerPattern(UnitType.LEVEL_ONE_PAINT_TOWER, towerLocation);
        }
        if (rc.canCompleteTowerPattern(UnitType.LEVEL_ONE_DEFENSE_TOWER, towerLocation)) {
            rc.completeTowerPattern(UnitType.LEVEL_ONE_DEFENSE_TOWER, towerLocation);
        }
    }

    // get center of map
    public static MapLocation getMapCenter(RobotController rc) {
        return new MapLocation(rc.getMapWidth() / 2, rc.getMapHeight() / 2);
    }

    // direction from robot toward center of map
    public static Direction directionToCenter(RobotController rc) {
        return rc.getLocation().directionTo(getMapCenter(rc));
    }

}
