package com.bananarepublic.service;

import com.bananarepublic.core.board.Board;
import com.bananarepublic.core.board.Intersection;
import com.bananarepublic.core.board.IntersectionId;
import com.bananarepublic.core.board.Path;
import com.bananarepublic.core.board.PathId;
import com.bananarepublic.core.game.GameState;
import com.bananarepublic.core.player.AbstractPlayer;
import com.bananarepublic.core.player.PlayerId;

import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;

public final class LongestRoadService {
    private static final int MIN_LONGEST_ROAD_LENGTH = 5;

    public Optional<PlayerId> updateLongestRoadLength(GameState state, PlayerId playerId, int roadLength) {
        Objects.requireNonNull(state, "state");
        Objects.requireNonNull(playerId, "playerId");
        state.getPlayerById(playerId).getStats().setLongestRoadLength(roadLength);
        return refreshLongestRoadOwner(state);
    }

    public Optional<PlayerId> refreshLongestRoadOwner(GameState state) {
        Objects.requireNonNull(state, "state");

        PlayerId currentOwnerId = state.getLongestRoadOwnerId();
        if (currentOwnerId != null) {
            AbstractPlayer currentOwner = state.getPlayerById(currentOwnerId);
            int currentLength = currentOwner.getStats().getLongestRoadLength();
            if (currentOwner.getStats().hasRoadAtLeastLength(MIN_LONGEST_ROAD_LENGTH)) {
                PlayerId ownerId = currentOwnerId;
                int ownerLength = currentLength;
                for (AbstractPlayer player : state.getPlayers()) {
                    int playerLength = player.getStats().getLongestRoadLength();
                    if (playerLength > ownerLength) {
                        ownerId = player.getId();
                        ownerLength = playerLength;
                    }
                }
                state.setLongestRoadOwnerId(ownerId);
                return Optional.of(ownerId);
            }
        }

        return chooseUniqueLeader(state, MIN_LONGEST_ROAD_LENGTH)
                .map(ownerId -> {
                    state.setLongestRoadOwnerId(ownerId);
                    return ownerId;
                })
                .or(() -> {
                    state.clearLongestRoadOwner();
                    return Optional.empty();
                });
    }

    public int computeLongestRoadLength(Board board, PlayerId playerId) {
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(playerId, "playerId");

        // Build adjacency: for each intersection, which owned paths connect to it
        Map<IntersectionId, Set<PathId>> adjacency = new HashMap<>();
        Set<PathId> allOwnedPaths = new HashSet<>();

        for (Path path : board.getPaths().values()) {
            boolean owned = path.getPipe()
                    .map(p -> p.getOwnerId().equals(playerId))
                    .orElse(false);
            if (!owned) {
                continue;
            }
            allOwnedPaths.add(path.getId());
            adjacency.computeIfAbsent(path.getFirstIntersectionId(), k -> new HashSet<>()).add(path.getId());
            adjacency.computeIfAbsent(path.getSecondIntersectionId(), k -> new HashSet<>()).add(path.getId());
        }

        if (allOwnedPaths.isEmpty()) {
            return 0;
        }

        // DFS from each path in both directions to find the longest chain
        int longest = 0;
        for (PathId startPathId : allOwnedPaths) {
            Path startPath = board.getPaths().get(startPathId);
            if (startPath == null) continue;

            Set<PathId> visited = new HashSet<>();
            visited.add(startPathId);

            int lenA = 1 + dfsLongest(startPath.getFirstIntersectionId(), visited, board, playerId, adjacency);
            int lenB = 1 + dfsLongest(startPath.getSecondIntersectionId(), visited, board, playerId, adjacency);
            longest = Math.max(longest, Math.max(lenA, lenB));
        }

        return longest;
    }

    public void updateAllRoadLengths(Board board, GameState state) {
        Objects.requireNonNull(board, "board");
        Objects.requireNonNull(state, "state");
        for (AbstractPlayer player : state.getPlayers()) {
            int length = computeLongestRoadLength(board, player.getId());
            player.getStats().setLongestRoadLength(length);
        }
        refreshLongestRoadOwner(state);
    }

    private int dfsLongest(IntersectionId currentId, Set<PathId> visited, Board board, PlayerId playerId, Map<IntersectionId, Set<PathId>> adjacency) {
        // Check if blocked by opponent building
        Intersection intersection = board.getIntersections().get(currentId);
        if (intersection != null && intersection.hasBuilding()) {
            var building = intersection.getBuilding().orElse(null);
            if (building != null && !building.getOwnerId().equals(playerId)) {
                return 0; // blocked by opponent
            }
        }

        int maxDepth = 0;
        for (PathId nextPathId : adjacency.getOrDefault(currentId, Collections.emptySet())) {
            if (visited.contains(nextPathId)) {
                continue;
            }

            Path nextPath = board.getPaths().get(nextPathId);
            if (nextPath == null) continue;

            IntersectionId otherId = nextPath.getFirstIntersectionId().equals(currentId)
                    ? nextPath.getSecondIntersectionId()
                    : nextPath.getFirstIntersectionId();

            visited.add(nextPathId);
            int depth = 1 + dfsLongest(otherId, visited, board, playerId, adjacency);
            visited.remove(nextPathId);

            maxDepth = Math.max(maxDepth, depth);
        }

        return maxDepth;
    }

    private Optional<PlayerId> chooseUniqueLeader(GameState state, int minimumLength) {
        PlayerId leaderId = null;
        int leaderLength = minimumLength - 1;
        boolean tied = false;

        for (AbstractPlayer player : state.getPlayers()) {
            int length = player.getStats().getLongestRoadLength();
            if (!player.getStats().hasRoadAtLeastLength(minimumLength)) {
                continue;
            }
            if (length > leaderLength) {
                leaderId = player.getId();
                leaderLength = length;
                tied = false;
            } else if (length == leaderLength) {
                tied = true;
            }
        }

        if (leaderId == null || tied) {
            return Optional.empty();
        }
        return Optional.of(leaderId);
    }
}
