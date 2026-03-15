package holo_oven;

import battlecode.common.*;

public class Soldier extends Robot {

    public enum SoldierState {
        ADVANCE,
        ATTACK
    }

    // paint if possible, given MapInfo and/or MapLocation
    public static void paint(RobotController rc, MapInfo paintTile, MapLocation paintLocation) throws GameActionException {
        if (paintTile.getPaint() == PaintType.EMPTY && rc.canAttack(paintLocation)) {
            rc.attack(paintLocation);
        } 
        else if ((!paintTile.getPaint().isEnemy()) && paintTile.getMark() != paintTile.getPaint() && paintTile.getMark() != PaintType.EMPTY && rc.canAttack(paintLocation)) {
            boolean useSecondaryColor = paintTile.getMark() == PaintType.ALLY_SECONDARY;
            rc.attack(paintLocation, useSecondaryColor);
        }
    }

    public static void paint(RobotController rc, MapInfo paintTile) throws GameActionException {
        MapLocation paintLocation = paintTile.getMapLocation();
        paint(rc, paintTile, paintLocation);
    }

    public static void paint(RobotController rc, MapLocation paintLocation) throws GameActionException {
        MapInfo paintTile = rc.senseMapInfo(rc.getLocation());
        paint(rc, paintTile, paintLocation);
    }
    
    public static void attackEnemyTower(RobotController rc, MapInfo[] nearbyTiles) throws GameActionException {
        for (MapInfo nearbyTile : nearbyTiles) {
            if (nearbyTile.hasRuin() && rc.canSenseRobotAtLocation(nearbyTile.getMapLocation())
                    && !rc.senseRobotAtLocation(nearbyTile.getMapLocation()).getTeam().equals(rc.getTeam())) {
                RobotPlayer.enemyTower = nearbyTile;
                if (rc.canAttack(nearbyTile.getMapLocation())) {
                    rc.attack(nearbyTile.getMapLocation());
                } else {
                    Direction dir = Pathfinding.pathfindAttack(rc, nearbyTile.getMapLocation());
                    if (dir != null) {
                        rc.move(dir);
                    }
                }
                return;
            }
        }
        RobotPlayer.enemyTower = null;

        // if no tower target, keep pressuring nearest visible enemy
        RobotInfo[] visibleEnemies = rc.senseNearbyRobots(Constants.GLOBAL_SENSE_RADIUS, rc.getTeam().opponent());
        RobotInfo chaseTarget = null;
        int bestDist = Integer.MAX_VALUE;
        for (RobotInfo enemy : visibleEnemies) {
            int dist = rc.getLocation().distanceSquaredTo(enemy.getLocation());
            if (enemy.getType().isTowerType()) {
                dist -= 8;
            }
            if (dist < bestDist) {
                bestDist = dist;
                chaseTarget = enemy;
            }
        }

        if (chaseTarget != null) {
            Direction dir = Pathfinding.pathfindAttack(rc, chaseTarget.getLocation());
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

    
    //paints the tile underfoot and nearby paintable tiles within action radius
    public static void paintAroundSelf(RobotController rc) throws GameActionException {
        // paint tile underfoot first
        MapLocation myLoc = rc.getLocation();
        if (rc.canAttack(myLoc)) {
            MapInfo myTile = rc.senseMapInfo(myLoc);
            paint(rc, myTile, myLoc);
        }

        // paint nearby tiles within action radius
        MapInfo[] nearby = rc.senseNearbyMapInfos(rc.getType().actionRadiusSquared);
        for (MapInfo tile : nearby) {
            if (!rc.isActionReady()) break;
            MapLocation tileLoc = tile.getMapLocation();
            if (tile.isPassable() && rc.canAttack(tileLoc)) {
                paint(rc, tile, tileLoc);
            }
        }
    }


    // go to the nearest allied tower when health or paint is low, return true if moved
    public static boolean retreat(RobotController rc) throws GameActionException {
        boolean lowHealth = Robot.hasLowHealth(rc, Constants.RETREAT_HEALTH_PERCENTAGE);
        boolean lowPaint = Robot.hasLowPaint(rc, Constants.RETREAT_PAINT_PERCENTAGE);

        if (!lowHealth && !lowPaint) {
            return false;
        }

        Direction dir = Pathfinding.returnToTower(rc);
        if (dir != null && rc.canMove(dir)) {
            rc.move(dir);
            return true;
        }

        return false;
    }


    //go towards unpainted areas, painting tiles along the way
    public static boolean explore(RobotController rc) throws GameActionException {
        // paint around before moving
        paintAroundSelf(rc);

        // move toward unpainted areas
        Direction dir = Pathfinding.exploreUnpainted(rc);
        if (dir != null && rc.canMove(dir)) {
            rc.move(dir);

            // paint around new location
            paintAroundSelf(rc);
            return true;
        }

        return false;
    }
}