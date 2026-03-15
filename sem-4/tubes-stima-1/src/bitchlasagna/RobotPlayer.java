package bitchlasagna;

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
        // 1. retreat if low HP or low paint
        if (Soldier.retreat(rc)) {
            return;
        }

        // 2. check for unclaimed ruin, build a tower
        MapLocation ruinLoc = Sensing.findUnclaimedRuin(rc, Constants.SOLDIER_RUIN_SEARCH_RADIUS);

        if (ruinLoc != null) {
            // mark tower if robot can sense
            if (rc.canSenseLocation(ruinLoc)) {
                UnitType towerType;
                
                if (Math.abs(ruinLoc.x * 31 + ruinLoc.y) % Constants.TOTAL_TOWER_RATIO < Constants.PAINT_TOWER_RATIO) {  // 'random function' for each coordinate
                    towerType = UnitType.LEVEL_ONE_PAINT_TOWER;
                }
                else {
                    towerType = UnitType.LEVEL_ONE_MONEY_TOWER;
                }

                Robot.markTower(rc, towerType, ruinLoc);

                // find nearest unpainted pattern tile to move toward
                MapLocation moveTarget = ruinLoc;
                for (MapInfo patternTile : rc.senseNearbyMapInfos(ruinLoc, Constants.RUIN_PATTERN_RADIUS_SQUARED)) {
                    if (patternTile.getMark() != patternTile.getPaint() && patternTile.getMark() != PaintType.EMPTY && !patternTile.hasRuin()) {
                        moveTarget = patternTile.getMapLocation();
                        break;
                    }
                }

                // move toward unpainted pattern tile so we can attack it
                if (rc.isMovementReady()) {
                    Direction dir = Pathfinding.pathfind(rc, moveTarget);
                    if (dir != null && rc.canMove(dir)) {
                        rc.move(dir);
                    }
                }

                // paint pattern tiles within attack range
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

                // try to complete the tower
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

        // 3. check for enemy towers, switch state to ATTACK
        RobotInfo enemy = Sensing.towerInRange(rc, Constants.SOLDIER_ENEMY_DETECT_RADIUS, false);

        if (enemy != null || soldierState == Soldier.SoldierState.ATTACK) {
            soldierState = Soldier.SoldierState.ATTACK;

            if (rc.isActionReady()) {
                Soldier.attackEnemyTower(rc, tilesInRange);
            }

            if (rc.isActionReady()) {
                Soldier.paintAroundSelf(rc);
            }

            // if no enemies visible, return to ADVANCE
            if (enemy == null && RobotPlayer.enemyTower == null) {
                soldierState = Soldier.SoldierState.ADVANCE;
            }

            return;
        }

        // 4. state ADVANCE, use explore()
        soldierState = Soldier.SoldierState.ADVANCE;

        if (!Soldier.explore(rc)) {
            // explore is bool, if false do wallFollow
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
        // 1. for round 1 spawn soldier
        if (rc.getRoundNum() == 1) {
            // spawn a soldier bot
            for (Direction dir : Constants.DIRECTIONS) {
                Tower.createSoldier(rc, dir);
            }
        }

        // 2. attack if enemy in range
        if (rc.isActionReady()) {
            Tower.attackLowestRobot(rc);
        }

        // 3. use AoE attack if more than 3 enemies nearby
        if (rc.isActionReady()) {
            RobotInfo[] enemy = rc.senseNearbyRobots(rc.getType().actionRadiusSquared, rc.getTeam().opponent());

            if (enemy.length >= Constants.TOWER_AOE_ENEMY_THRESHOLD) {
                Tower.aoeAttack(rc);
            }
        }

        // 4. upgrade tower if its after 200 round
        if (rc.getRoundNum() >= Constants.TOWER_UPGRADE_ROUND) {
            Tower.upgradeSelf(rc);
        }

        // 5. spawn unit, but reserve some chip for building a tower
        if (rc.getChips() >= Constants.TOWER_BUILD_CHIP_RESERVE) {
            if (Tower.spawnUnit(rc, spawnCount)) {
                spawnCount++;
            }
        }
    }

}