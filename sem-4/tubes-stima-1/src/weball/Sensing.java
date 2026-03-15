package weball;

import battlecode.common.*;

import java.util.ArrayList;
import java.util.List;

public class Sensing {

    /**
     * Mencari enemy dengan HP terendah dalam action radius yang bisa diserang.
     * Mengembalikan null jika tidak ada enemy dalam jangkauan.
     */
    public static RobotInfo findNearestLowestHP(RobotController rc) throws GameActionException {
        RobotInfo[] enemies = rc.senseNearbyRobots(rc.getType().actionRadiusSquared, rc.getTeam().opponent());
        RobotInfo targetBot = null;
        int minHealth = Constants.UNSET_DISTANCE;
        for (RobotInfo robot : enemies) {
            int hp = robot.getHealth();
            if (minHealth == Constants.UNSET_DISTANCE || minHealth > hp) {
                targetBot = robot;
                minHealth = hp;
            }
        }
        return targetBot;
    }

    /**
     * Mengecek apakah pattern tower masih perlu diisi.
     * Mengembalikan true jika ada tile kosong atau paint ally yang tidak sesuai mark.
     */
    public static boolean needFilling(RobotController rc, MapLocation towerLocation) throws GameActionException {
        for (MapInfo tile : rc.senseNearbyMapInfos(towerLocation, Constants.RUIN_PATTERN_RADIUS_SQUARED)) {
            if (!tile.hasRuin() && (tile.getPaint() == PaintType.EMPTY ||
                    tile.getPaint().isAlly() && tile.getMark() != tile.getPaint())) {
                return true;
            }
        }
        return false;
    }

    /**
     * Mengecek apakah semua tile pada pattern sudah valid untuk membangun tower.
     * Mengembalikan false jika ada tile kosong, paint/mark tidak cocok, enemy paint, atau ada robot di ruin.
     */
    public static boolean canBuildTower(RobotController rc, MapLocation towerLocation) throws GameActionException {
        for (MapInfo tile : rc.senseNearbyMapInfos(towerLocation, Constants.RUIN_PATTERN_RADIUS_SQUARED)) {
            if (tile.hasRuin()) {
                if (rc.canSenseRobotAtLocation(tile.getMapLocation())) {
                    return false;
                }
            } else if ((tile.getMark() == PaintType.EMPTY
                    || tile.getMark() != tile.getPaint()
                    || tile.getPaint().isEnemy())) {
                return false;
            }
        }
        return true;
    }

    /**
     * Mencari tile pada range yang bisa di-paint untuk pattern tower.
     * Mengembalikan null jika tidak ada tile yang memenuhi syarat.
     */
    public static MapInfo findPaintableTile(RobotController rc, MapLocation location, int range) throws GameActionException {
        for (MapInfo tile : rc.senseNearbyMapInfos(location, range)) {
            if (rc.canPaint(tile.getMapLocation()) &&
                    (tile.getPaint() == PaintType.EMPTY ||
                            tile.getMark() != tile.getPaint() && tile.getMark() != PaintType.EMPTY)) {
                return tile;
            }
        }
        return null;
    }

    /**
     * Mengambil daftar adjacent tile yang passable dan belum memiliki paint.
     * Daftar ini digunakan sebagai kandidat move ke area kosong.
     */
    public static List<MapInfo> getMovableEmptyTiles(RobotController rc) throws GameActionException {
        MapInfo[] adjTiles = rc.senseNearbyMapInfos(Constants.ADJACENT_RADIUS_SQUARED);
        List<MapInfo> validAdj = new ArrayList<>();
        for (MapInfo tile : adjTiles) {
            if (tile.getPaint() == PaintType.EMPTY && tile.isPassable()) {
                validAdj.add(tile);
            }
        }
        return validAdj;
    }

    /**
     * Mencari robot tower apa pun di dalam range.
     * Mengembalikan null jika tidak ada tower yang terdeteksi.
     */
    public static RobotInfo towerInRange(RobotController rc, int range) throws GameActionException {
        RobotInfo[] botsInRange = rc.senseNearbyRobots(range);
        for (RobotInfo bot : botsInRange) {
            if (bot.getType().isTowerType()) {
                return bot;
            }
        }
        return null;
    }

    /**
     * Mencari robot tower di dalam range berdasarkan tim.
     * Jika ally true, cari tower ally; jika false, cari tower enemy.
     */
    public static RobotInfo towerInRange(RobotController rc, int range, boolean ally) throws GameActionException {
        RobotInfo[] botsInRange;
        if (ally) {
            botsInRange = rc.senseNearbyRobots(range, rc.getTeam());
        } else {
            botsInRange = rc.senseNearbyRobots(range, rc.getTeam().opponent());
        }
        for (RobotInfo bot : botsInRange) {
            if (bot.getType().isTowerType()) {
                return bot;
            }
        }
        return null;
    }

    /**
     * Mencari tile pertama pada nearbyTiles yang memiliki enemy paint.
     */
    public static MapInfo findEnemyPaint(RobotController rc, MapInfo[] nearbyTiles) throws GameActionException {
        for (MapInfo tile : nearbyTiles) {
            if (tile.getPaint().isEnemy()) {
                return tile;
            }
        }
        return null;
    }

    /**
     * Menghitung jumlah empty tile di sekitar center dalam adjacent radius.
     * Hanya menghitung tile yang passable dan tidak ditempati robot.
     */
    public static int countEmptyAround(RobotController rc, MapLocation center) throws GameActionException {
        MapInfo[] nearbyTiles = rc.senseNearbyMapInfos(center, Constants.ADJACENT_RADIUS_SQUARED);
        int count = 0;
        for (MapInfo tile : nearbyTiles) {
            if (tile.getPaint() == PaintType.EMPTY && tile.isPassable()
                    && !rc.canSenseRobotAtLocation(tile.getMapLocation())) {
                count++;
            }
        }
        return count;
    }

    /**
     * Memilih direction dengan score terendah untuk mendekati target.
     * Nilai score dihitung oleh fungsi scoreDirection.
     */
    public static Direction getGreedyDirectionToward(RobotController rc, MapLocation target) throws GameActionException {
        if (target == null) {
            return null;
        }

        MapLocation current = rc.getLocation();
        Direction bestDir = null;
        int minScore = Integer.MAX_VALUE;

        for (Direction direction : Constants.DIRECTIONS) {
            int score = scoreDirection(rc, current, target, direction);
            if (score < minScore) {
                minScore = score;
                bestDir = direction;
            }
        }

        if (bestDir == null || minScore >= Constants.GREEDY_BLOCKED_PENALTY) {
            return null;
        }

        return bestDir;
    }

    /**
     * Melakukan move ke direction terbaik menuju target berdasarkan greedy score.
     * Mengembalikan true jika move berhasil, false jika tidak bisa move.
     */
    public static boolean moveGreedyToward(RobotController rc, MapLocation target) throws GameActionException {
        if (!rc.isMovementReady()) {
            return false;
        }

        Direction bestDir = getGreedyDirectionToward(rc, target);
        if (bestDir != null && rc.canMove(bestDir)) {
            rc.move(bestDir);
            return true;
        }
        return false;
    }

    /**
     * Menghitung score untuk move ke direction tertentu.
     * Formula score: distance ke target * 10 + penalty/bonus paint.
     */
    private static int scoreDirection(RobotController rc, MapLocation current, MapLocation target, Direction direction) throws GameActionException {
        if (!rc.canMove(direction)) {
            return Constants.GREEDY_BLOCKED_PENALTY;
        }

        MapLocation next = current.add(direction);
        int score = next.distanceSquaredTo(target) * 10;

        if (rc.canSenseLocation(next)) {
            MapInfo tile = rc.senseMapInfo(next);
            PaintType paint = tile.getPaint();

            if (paint.isEnemy()) {
                score += Constants.GREEDY_ENEMY_PAINT_PENALTY;
            } else if (paint == PaintType.EMPTY) {
                score += Constants.GREEDY_EMPTY_PAINT_PENALTY;
            } else if (paint.isAlly()) {
                score += Constants.GREEDY_ALLY_PAINT_BONUS;
            }
        }

        return score;
    }

    /**
     * Mencari unclaimed ruin terdekat
     */
    public static MapLocation findUnclaimedRuin(RobotController rc, int range) throws GameActionException {
        MapLocation[] ruins = rc.senseNearbyRuins(range);
        MapLocation closest = null;
        int minDist = Constants.UNSET_DISTANCE;
        for (MapLocation ruin : ruins) {
            if (!rc.canSenseRobotAtLocation(ruin)) {
                int dist = rc.getLocation().distanceSquaredTo(ruin);
                if (minDist == Constants.UNSET_DISTANCE || dist < minDist) {
                    closest = ruin;
                    minDist = dist;
                }
            }
        }
        return closest;
    }
}