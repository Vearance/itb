package holo_oven;

import battlecode.common.*;

import java.util.List;

public class Pathfinding {
	// wall exploration
	static Direction lastExploreDir = null;

	/**
	 * Menentukan arah gerak terbaik menuju target dengan mempertimbangkan jarak dan biaya paint.
	 */
	public static Direction pathfind(RobotController rc, MapLocation target) throws GameActionException {
		if (target == null) {
			return null;
		}

		int minDist = Constants.UNSET_DISTANCE;
		PaintType bestPaint = PaintType.EMPTY;
		MapLocation curLoc = rc.getLocation();
		MapInfo bestLoc = null;

		for (Direction dir : Constants.DIRECTIONS) {
			if (rc.canMove(dir)) {
				MapInfo adjLoc = rc.senseMapInfo(curLoc.add(dir));
				int dist = adjLoc.getMapLocation().distanceSquaredTo(target);
				PaintType adjPaint = adjLoc.getPaint();

				if ((dist < minDist || minDist == Constants.UNSET_DISTANCE) && canAffordMove(rc, adjPaint)) {
					minDist = dist;
					bestPaint = adjPaint;
					bestLoc = adjLoc;
				} else if (dist == minDist) {
					PaintType tiePaint = adjLoc.getPaint();
					if ((bestPaint.isEnemy() && !tiePaint.isEnemy() ||
							bestPaint == PaintType.EMPTY && tiePaint.isAlly()) &&
							canAffordMove(rc, adjPaint)) {
						minDist = dist;
						bestPaint = adjLoc.getPaint();
						bestLoc = adjLoc;
					}
				}
			}
		}

		if (minDist != Constants.UNSET_DISTANCE && bestLoc != null) {
			return curLoc.directionTo(bestLoc.getMapLocation());
		}

		return null;
	}

	/**
	 * Greedy pathfinding yang lebih agresif: tetap mendekati target, namun
	 * memberi prioritas pada posisi yang membuka peluang menyerang enemy.
	 */
	public static Direction pathfindAttack(RobotController rc, MapLocation target) throws GameActionException {
		if (target == null) {
			return null;
		}

		MapLocation curLoc = rc.getLocation();
		RobotInfo[] enemies = rc.senseNearbyRobots(Constants.GLOBAL_SENSE_RADIUS, rc.getTeam().opponent());
		int actionRadius = rc.getType().actionRadiusSquared;

		Direction bestDir = null;
		int minScore = Integer.MAX_VALUE;

		for (Direction dir : Constants.DIRECTIONS) {
			if (!rc.canMove(dir)) {
				continue;
			}

			MapLocation next = curLoc.add(dir);
			MapInfo nextInfo = rc.senseMapInfo(next);
			PaintType nextPaint = nextInfo.getPaint();

			int score = next.distanceSquaredTo(target) * 10;

			if (nextPaint.isEnemy()) {
				score += Constants.ATTACK_MODE_ENEMY_PAINT_BONUS;
			} else if (nextPaint == PaintType.EMPTY) {
				score += Constants.ATTACK_MODE_EMPTY_PAINT_PENALTY;
			} else if (nextPaint.isAlly()) {
				score += Constants.ATTACK_MODE_ALLY_PAINT_PENALTY;
			}

			if (!canAffordMove(rc, nextPaint)) {
				score += Constants.GREEDY_BLOCKED_PENALTY;
			}

			for (RobotInfo enemy : enemies) {
				int distAfterMove = next.distanceSquaredTo(enemy.getLocation());
				if (enemy.getType().isTowerType()) {
					if (distAfterMove <= actionRadius) {
						score -= Constants.ATTACK_MODE_IN_RANGE_TOWER_BONUS;
					}
					score -= Constants.ATTACK_MODE_TOWER_PROXIMITY_WEIGHT / (distAfterMove + 1);
				} else {
					if (distAfterMove <= actionRadius) {
						score -= Constants.ATTACK_MODE_IN_RANGE_UNIT_BONUS;
					}
					score -= Constants.ATTACK_MODE_UNIT_PROXIMITY_WEIGHT / (distAfterMove + 1);
				}
			}

			if (score < minScore) {
				minScore = score;
				bestDir = dir;
			}
		}

		return bestDir;
	}

	/**
	 * Menentukan arah gerak dasar: maju ke target, atau alternatif kiri/kanan, lalu arah acak yang valid.
	 */
	public static Direction originalPathfind(RobotController rc, MapLocation target) throws GameActionException {
		if (target == null) {
			return null;
		}

		Direction toTarget = rc.getLocation().directionTo(target);
		Direction left = toTarget.rotateLeft();
		Direction right = toTarget.rotateRight();

		if (rc.canMove(toTarget)) {
			return toTarget;
		} else if (rc.canMove(left)) {
			return left;
		} else if (rc.canMove(right)) {
			return right;
		}

		for (Direction direction : Direction.allDirections()) {
			if (rc.canMove(direction)) {
				return direction;
			}
		}

		return null;
	}

	/**
	 * Menentukan arah menuju target dengan memprioritaskan tile dengan paint ally sebelum tile lain.
	 */
	public static Direction paintedPathfind(RobotController rc, MapLocation target) throws GameActionException {
		if (target == null) {
			return null;
		}

		Direction toTarget = rc.getLocation().directionTo(target);
		Direction left = toTarget.rotateLeft();
		Direction right = toTarget.rotateRight();

		if (canMoveOnAlly(rc, toTarget)) {
			return toTarget;
		} else if (canMoveOnAlly(rc, left)) {
			return left;
		} else if (canMoveOnAlly(rc, right)) {
			return right;
		}

		for (Direction direction : Direction.allDirections()) {
			if (canMoveOnAlly(rc, direction)) {
				return direction;
			}
		}

		for (Direction direction : Direction.allDirections()) {
			if (rc.canMove(direction)) {
				return direction;
			}
		}

		return null;
	}

	/**
	 * Mengembalikan arah untuk kembali ke tower sekutu terdekat yang terdeteksi.
	 */
	public static Direction returnToTower(RobotController rc) throws GameActionException {
		RobotInfo alliedTower = Sensing.towerInRange(rc, Constants.GLOBAL_SENSE_RADIUS, true);
		if (alliedTower == null) {
			return null;
		}
		return pathfind(rc, alliedTower.getLocation());
	}

	/**
	 * Memecahkan tie antar kandidat tile kosong dengan bobot berdasarkan banyaknya ruang kosong di sekitarnya.
	 */
	public static MapLocation tiebreakUnpainted(RobotController rc, List<MapInfo> validAdjacent) throws GameActionException {
		int totalWeight = 0;
		int tileCount = validAdjacent.size();
		int[] weightedAdj = new int[tileCount];

		for (int i = 0; i < tileCount; i++) {
			MapLocation adjLoc = validAdjacent.get(i).getMapLocation();
			Direction fromRobot = rc.getLocation().directionTo(adjLoc);
			MapLocation behind = adjLoc.add(fromRobot);
			totalWeight += Sensing.countEmptyAround(rc, behind);
			weightedAdj[i] = totalWeight;
		}

		if (totalWeight == 0) {
			Direction dir = wallFollow(rc);
			if (dir != null) {
				return rc.getLocation().add(dir);
			}
			return rc.getLocation();
		}

		int rand = (int) (Math.random() * totalWeight);
		for (int i = 0; i < tileCount; i++) {
			if (rand < weightedAdj[i]) {
				return validAdjacent.get(i).getMapLocation();
			}
		}

		return validAdjacent.get(0).getMapLocation();
	}

	/**
	 * Memilih arah eksplorasi ke area kosong/tanpa paint, dengan fallback ke tile yang masih bisa dilalui.
	 */
	public static Direction exploreUnpainted(RobotController rc) throws GameActionException {
		List<MapInfo> validAdj = Sensing.getMovableEmptyTiles(rc);
		if (validAdj.isEmpty()) {
			MapLocation curLoc = rc.getLocation();
			for (Direction direction : Constants.DIRECTIONS) {
				if (rc.canMove(direction)) {
					validAdj.add(rc.senseMapInfo(curLoc.add(direction)));
				}
			}
		}

		if (validAdj.isEmpty()) {
			return null;
		}

		Direction moveDir = rc.getLocation().directionTo(tiebreakUnpainted(rc, validAdj));
		if (rc.canMove(moveDir)) {
			return moveDir;
		}
		return null;
	}

	/**
	 * Memilih arah untuk keluar dari kebuntuan dengan bergerak menuju sudut peta yang berlawanan.
	 */
	public static Direction getUnstuck(RobotController rc) throws GameActionException {
		MapLocation current = rc.getLocation();
		int targetX = current.x < rc.getMapWidth() / 2 ? rc.getMapWidth() - 1 : 0;
		int targetY = current.y < rc.getMapHeight() / 2 ? rc.getMapHeight() - 1 : 0;
		MapLocation oppositeCorner = new MapLocation(targetX, targetY);
		return pathfind(rc, oppositeCorner);
	}

	/**
	 * Wall-following algo: try last heading, rotate right if blocked, returns a movable direction, or null if stuck.
	 */
	public static Direction wallFollow(RobotController rc) throws GameActionException {
		if (lastExploreDir == null) {
			// initialize heading toward center of map
			lastExploreDir = Robot.directionToCenter(rc);
		}

		// try current heading, then rotate right up to 7 times
		Direction dir = lastExploreDir;
		for (int i = 0; i < 8; i++) {
			if (rc.canMove(dir)) {
				lastExploreDir = dir;
				return dir;
			}
			dir = dir.rotateRight();
		}
		return null;
	}

	/**
	 * Mengecek apakah robot bisa bergerak ke arah tertentu dan tile tujuan memiliki paint ally.
	 */
	private static boolean canMoveOnAlly(RobotController rc, Direction direction) throws GameActionException {
		if (!rc.canMove(direction)) {
			return false;
		}
		MapLocation next = rc.getLocation().add(direction);
		return rc.senseMapInfo(next).getPaint().isAlly();
	}

	/**
	 * Mengecek apakah robot masih memiliki paint yang cukup setelah melakukan pergerakan.
	 */
	private static boolean canAffordMove(RobotController rc, PaintType paintType) {
		int remainPaint = rc.getPaint() - paintLossFor(paintType);
		return remainPaint > Constants.MIN_REMAINING_PAINT_AFTER_MOVE;
	}

	/**
	 * Menghitung kehilangan paint akibat melangkah pada paint type tertentu.
	 */
	private static int paintLossFor(PaintType paintType) {
		if (paintType.isEnemy()) {
			return Constants.ENEMY_PAINT_MOVE_LOSS;
		}
		if (paintType == PaintType.EMPTY) {
			return Constants.EMPTY_PAINT_MOVE_LOSS;
		}
		return 0;
	}
}
