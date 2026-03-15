package weball;

import battlecode.common.*;

public class Tower {
    
    public static void createSoldier(RobotController rc, Direction spawnDirection) throws GameActionException {
        MapLocation spawnLoc = rc.getLocation().add(spawnDirection);
        if (rc.canBuildRobot(UnitType.SOLDIER, spawnLoc)) {
            rc.buildRobot(UnitType.SOLDIER, spawnLoc);
        }
    }

    public static void createMopper(RobotController rc, Direction spawnDirection) throws GameActionException {
        MapLocation spawnLoc = rc.getLocation().add(spawnDirection);
        if (rc.canBuildRobot(UnitType.MOPPER, spawnLoc)) {
            rc.buildRobot(UnitType.MOPPER, spawnLoc);
        }
    }

    public static void createSplasher(RobotController rc, Direction spawnDirection) throws GameActionException {
        MapLocation spawnLoc = rc.getLocation().add(spawnDirection);
        if (rc.canBuildRobot(UnitType.SPLASHER, spawnLoc)) {
            rc.buildRobot(UnitType.SPLASHER, spawnLoc);
        }
    }

    public static boolean isStartSquareCovered(RobotController rc, Direction spawnDirection) throws GameActionException {
        return rc.senseMapInfo(rc.getLocation().add(spawnDirection)).getPaint().isEnemy();
    }

    // attack lowest HP robot
    public static void attackLowestRobot(RobotController rc) throws GameActionException {
        RobotInfo target = Sensing.findNearestLowestHP(rc);
        if (target != null) {
            if (rc.canAttack(target.getLocation())) {
                rc.attack(target.getLocation());
            }
        }
    }

    // tower do AOE attack
    public static void aoeAttack(RobotController rc) throws GameActionException {
        if (rc.canAttack(null)) {
            rc.attack(null);
        }
    }

    // upgrade self to next level
    public static void upgradeSelf(RobotController rc) throws GameActionException {
        if (rc.canUpgradeTower(rc.getLocation())) {
            rc.upgradeTower(rc.getLocation());
        }
    }

    // rotation spawn unit
    public static boolean spawnUnit(RobotController rc, int spawnCount) throws GameActionException {
        int cycle = spawnCount % Constants.TOTAL_SPAWN_RATIO;
        UnitType type;
        if (cycle < Constants.SOLDIER_RATIO) {
            type = UnitType.SOLDIER;
        } else if (cycle < Constants.SOLDIER_RATIO + Constants.MOPPER_RATIO) {
            type = UnitType.MOPPER;
        } else {
            type = UnitType.SPLASHER;
        }

        for (Direction dir : Constants.DIRECTIONS) {
            MapLocation spawnLoc = rc.getLocation().add(dir);
            if (rc.canBuildRobot(type, spawnLoc)) {
                if (type == UnitType.SOLDIER) createSoldier(rc, dir);
                else if (type == UnitType.MOPPER) createMopper(rc, dir);
                else createSplasher(rc, dir);
                return true;
            }
        }
        return false;
    }

    // attack by territory
    public static void towerAttack(RobotController rc) throws GameActionException {
        RobotInfo[] enemies = rc.senseNearbyRobots(rc.getType().actionRadiusSquared, rc.getTeam().opponent());
        
        if (enemies.length == 0) return;

        // If many enemies, do AOE attack
        if (enemies.length >= 3 && rc.canAttack(null)) {
            rc.attack(null);
            return;
        }

        RobotInfo bestTarget = null;
        int maxScore = -1;

        for (RobotInfo enemy : enemies) {
            if (rc.canAttack(enemy.getLocation())) {
                int score = 0;
                MapInfo tile = rc.senseMapInfo(enemy.getLocation());
                
                // focus on enemy in territory
                if (tile.getPaint().isAlly()) {
                    score += 50; 
                }
                
                // Prioritize with high paint enemy
                score += enemy.getPaintAmount();

                if (score > maxScore) {
                    maxScore = score;
                    bestTarget = enemy;
                }
            }
        }

        if (bestTarget != null) {
            rc.attack(bestTarget.getLocation());
        } else {
            // fallback: attack lowest HP enemy
            attackLowestRobot(rc); 
        }
    }

    // Spawn based on condition
    public static boolean doSpawnUnit(RobotController rc, int spawnCount) throws GameActionException {
        MapInfo[] nearbyTiles = rc.senseNearbyMapInfos(rc.getType().actionRadiusSquared);
        int enemyPaintCount = 0;
        
        for (MapInfo tile : nearbyTiles) {
            if (tile.getPaint().isEnemy()) {
                enemyPaintCount++;
            }
        }

        UnitType typeToSpawn;

        // If there's a lot of enemy tiles, spawn moppers
        if (enemyPaintCount > 5) {
            typeToSpawn = UnitType.MOPPER;
        } 
        else {
            int cycle = spawnCount % Constants.TOTAL_SPAWN_RATIO;
            if (cycle < Constants.SOLDIER_RATIO) {
                typeToSpawn = UnitType.SOLDIER;
            } else if (cycle < Constants.SOLDIER_RATIO + Constants.MOPPER_RATIO) {
                typeToSpawn = UnitType.MOPPER;
            } else {
                typeToSpawn = UnitType.SPLASHER;
            }
        }

        // find empty tile for spawning
        for (Direction dir : Constants.DIRECTIONS) {
            MapLocation spawnLoc = rc.getLocation().add(dir);
            if (rc.canBuildRobot(typeToSpawn, spawnLoc)) {
                if (typeToSpawn == UnitType.SOLDIER) createSoldier(rc, dir);
                else if (typeToSpawn == UnitType.MOPPER) createMopper(rc, dir);
                else createSplasher(rc, dir);
                return true;
            }
        }
        
        return false;
    }
}