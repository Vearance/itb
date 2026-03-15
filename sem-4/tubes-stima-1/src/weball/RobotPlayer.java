package weball;

import battlecode.common.*;

// import java.util.Arrays;
// import java.util.HashMap;
// import java.util.HashSet;
// import java.util.Map;
// import java.util.Random;
// import java.util.Set;

public class RobotPlayer {
    static int turnCount = 0;
    static int spawnCount = 0;

    static MapInfo enemyTower = null;
    static MapInfo enemyTile = null;
    static Soldier.SoldierState soldierState = Soldier.SoldierState.ADVANCE;

    public static void run(RobotController rc) throws GameActionException {

        while (true) {
            turnCount++;

            try {
                switch (rc.getType()) {
                    case SOLDIER:
                        runSoldier(rc);
                        break;
                    case MOPPER:
                        runMopper(rc);
                        break;
                    case SPLASHER:
                        runSplasher(rc); 
                        break;
                    default:
                        runTower(rc);
                        break;
                }
            } catch (GameActionException e) {
                System.out.println(rc.getType() + " GameActionException");
                e.printStackTrace();
            } catch (Exception e) {
                System.out.println(rc.getType() + " Exception");
                e.printStackTrace();
            } finally {
                Clock.yield();  // end turn
            }

        }
    }


    public static void runSoldier(RobotController rc) throws GameActionException {

        // check ruin, build tower
        MapLocation ruinLoc = Sensing.findUnclaimedRuin(rc, Constants.SOLDIER_RUIN_SEARCH_RADIUS);
        if (ruinLoc != null) {
            // mark tower
            if (rc.canSenseLocation(ruinLoc)) {
                Robot.markTower(rc, UnitType.LEVEL_ONE_PAINT_TOWER, ruinLoc);

                // find nearest empty tile
                MapLocation moveTarget = ruinLoc;
                for (MapInfo patternTile : rc.senseNearbyMapInfos(ruinLoc, Constants.RUIN_PATTERN_RADIUS_SQUARED)) {
                    if (patternTile.getMark() != patternTile.getPaint() && patternTile.getMark() != PaintType.EMPTY && !patternTile.hasRuin()) {
                        moveTarget = patternTile.getMapLocation();
                        break;
                    }
                }

                // move toward empty tile
                if (rc.isMovementReady()) {
                    Direction dir = Pathfinding.pathfind(rc, moveTarget);
                    if (dir != null && rc.canMove(dir)) {
                        rc.move(dir);
                    }
                }

                // paint tiles
                if (rc.isActionReady()) {
                    for (MapInfo patternTile : rc.senseNearbyMapInfos(ruinLoc, Constants.RUIN_PATTERN_RADIUS_SQUARED)) {
                        if (!rc.isActionReady()) break;
                        if (patternTile.getMark() != patternTile.getPaint()
                                && patternTile.getMark() != PaintType.EMPTY) {
                            boolean useSecondaryColor = patternTile.getMark() == PaintType.ALLY_SECONDARY;
                            if (rc.canAttack(patternTile.getMapLocation())) {
                                rc.attack(patternTile.getMapLocation(), useSecondaryColor);
                            }
                        }
                    }
                }

                Robot.completeTower(rc, ruinLoc);
            }
            else {
                // move to ruins
                if (rc.isMovementReady()) {
                    Direction dir = Pathfinding.pathfind(rc, ruinLoc);
                    if (dir != null && rc.canMove(dir)) {
                        rc.move(dir);
                    }
                }
            }

            return;
        }

        MapInfo[] tilesInRange = rc.senseNearbyMapInfos();

        // check for enemy towers, switch state to ATTACK
        RobotInfo enemy = Sensing.towerInRange(rc, Constants.SOLDIER_ENEMY_DETECT_RADIUS, false);

        if (enemy != null || soldierState == Soldier.SoldierState.ATTACK) {
            soldierState = Soldier.SoldierState.ATTACK;

            if (rc.isActionReady()) {
                Soldier.attackEnemyTower(rc, tilesInRange);
            }

            if (rc.isActionReady()) {
                Soldier.paintAroundSelf(rc);
            }

            // if no enemies, return to ADVANCE
            if (enemy == null && RobotPlayer.enemyTower == null) {
                soldierState = Soldier.SoldierState.ADVANCE;
            }

            return;
        }

        // 4. state ADVANCE
        soldierState = Soldier.SoldierState.ADVANCE;

        if (!Soldier.explore(rc)) {
            if (rc.isMovementReady()) {
                Direction dir = Pathfinding.wallFollow(rc);

                if (dir != null && rc.canMove(dir)) {
                    rc.move(dir);
                    Soldier.paintAroundSelf(rc);
                }
            }
        }
    }

    public static void runMopper(RobotController rc) throws GameActionException {
        if (rc.isActionReady()) {
            Mopper.mopperAction(rc);
        }
        
        if (rc.isMovementReady()) {
            Mopper.moveToPerimeter(rc);
        }
    }

    public static void runSplasher(RobotController rc) throws GameActionException {
        if (rc.isActionReady() && rc.getPaint() >= Constants.SPLASHER_ATTACK_COST) {
            Splasher.splashAttack(rc);
        }

        if (rc.isMovementReady()) {
            Splasher.moveToEnemy(rc);
        }
    }

    public static void runTower(RobotController rc) throws GameActionException {
        // first, spawn soldier
        if (rc.getRoundNum() == 1) {
            for (Direction dir : Constants.DIRECTIONS) {
                Tower.createSoldier(rc, dir);
            }
        }

        // attack if enemy in range
        if (rc.isActionReady()) {
            Tower.attackLowestRobot(rc);
        }

        // use AoE attack if > 3 enemies nearby
        if (rc.isActionReady()) {
            RobotInfo[] enemy = rc.senseNearbyRobots(rc.getType().actionRadiusSquared, rc.getTeam().opponent());

            if (enemy.length >= Constants.TOWER_AOE_ENEMY_THRESHOLD) {
                Tower.aoeAttack(rc);
            }
        }

        // upgrade tower
        if (rc.getRoundNum() >= Constants.TOWER_UPGRADE_ROUND) {
            Tower.upgradeSelf(rc);
        }

        // 5. spawn unit, but reserve some chip
        if (rc.getChips() >= Constants.TOWER_BUILD_CHIP_RESERVE) {
            if (Tower.spawnUnit(rc, spawnCount)) {
                spawnCount++;
            }
        }
    }

}