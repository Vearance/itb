package weball;

import battlecode.common.*;

public class Constants {

    public static final Direction[] DIRECTIONS = {
        Direction.NORTH,
        Direction.NORTHEAST,
        Direction.EAST,
        Direction.SOUTHEAST,
        Direction.SOUTH,
        Direction.SOUTHWEST,
        Direction.WEST,
        Direction.NORTHWEST
    };

    public static final int ADJACENT_RADIUS_SQUARED = 2;
    public static final int RUIN_PATTERN_RADIUS_SQUARED = 8;
    public static final int GREEDY_BLOCKED_PENALTY = 100000;
    public static final int GREEDY_ENEMY_PAINT_PENALTY = 8;
    public static final int GREEDY_EMPTY_PAINT_PENALTY = 2;
    public static final int GREEDY_ALLY_PAINT_BONUS = -2;

    public static final int UNSET_DISTANCE = -1;
    public static final int GLOBAL_SENSE_RADIUS = -1;
    public static final int MIN_REMAINING_PAINT_AFTER_MOVE = 2;
    public static final int ENEMY_PAINT_MOVE_LOSS = 2;
    public static final int EMPTY_PAINT_MOVE_LOSS = 1;

    public static final int RETREAT_HEALTH_PERCENTAGE = 25;
    public static final int RETREAT_PAINT_PERCENTAGE = 20;

    // Mopper
    public static final int MOPPER_SENSE_RADIUS = 2;
    public static final int MOPPER_TRANSFER_AMOUNT = 20;
    public static final int MOPPER_MIN_PAINT_TO_TRANSFER = 30;
    public static final int MOPPER_LOW_ALLY_PAINT = 50;
    public static final int MOPPER_ALLY_PAINT_SCORE = 10;
    public static final int MOPPER_NEAR_LOW_ALLY_SCORE = 50;
    public static final int MOPPER_BORDER_TILE_SCORE = 5;
    public static final int MOPPER_ENEMY_PAINT_SCORE = 20;

    // Splasher
    public static final int SPLASHER_ATTACK_COST = 50;
    public static final int SPLASHER_ATTACK_RADIUS = 4;
    public static final int SPLASHER_INNER_RADIUS = 2;
    public static final int SPLASHER_TOWER_SCORE = 1000;
    public static final int SPLASHER_ENEMY_ROBOT_SCORE = 20;
    public static final int SPLASHER_ENEMY_PAINT_SCORE = 5;
    public static final int SPLASHER_EMPTY_PAINT_SCORE = 2;

    // Tower
    public static final int SOLDIER_RATIO = 2;
    public static final int MOPPER_RATIO = 1;
    public static final int SPLASHER_RATIO = 2;
    public static final int TOTAL_SPAWN_RATIO = SOLDIER_RATIO + MOPPER_RATIO + SPLASHER_RATIO;
    public static final int TOWER_AOE_ENEMY_THRESHOLD = 3;
    public static final int TOWER_UPGRADE_ROUND = 200;
    public static final int TOWER_BUILD_CHIP_RESERVE = 1200;

    // Soldier
    public static final int SOLDIER_RUIN_SEARCH_RADIUS = -1;
    public static final int SOLDIER_ENEMY_DETECT_RADIUS = -1;

}