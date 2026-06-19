from collections import deque

def retrograde_dynamic_programming(V, successors, predecessors, terminal_states):
    F = {v: "Unknown" for v in V}
    dtm = {v: None for v in V}
    Q = deque()
    out_degree = {v: len(successors[v]) for v in V}
    # Tracks the maximum DTM seen among Win successors, used to correctly
    # assign DTM to Loss nodes: DTM(Loss) = max(DTM of all Win successors) + 1
    max_win_dtm = {v: 0 for v in V}

    for v in terminal_states:
        F[v] = "Loss"
        dtm[v] = 0
        Q.append(v)

    while Q:
        v = Q.popleft()
        
        for p in predecessors[v]:
            if F[p] == "Unknown":
                if F[v] == "Loss":
                    # p can move to a Loss state -> p is Win.
                    # BFS processes Loss nodes in order of increasing DTM,
                    # so the first assignment here is already the minimum.
                    F[p] = "Win"
                    dtm[p] = dtm[v] + 1
                    Q.append(p)
                elif F[v] == "Win":
                    # One more successor of p has been resolved as Win.
                    out_degree[p] -= 1
                    max_win_dtm[p] = max(max_win_dtm[p], dtm[v])
                    if out_degree[p] == 0:
                        # All successors of p are Win -> p is Loss.
                        # DTM is the maximum Win-successor DTM + 1, because
                        # the losing side will stall by picking the move that
                        # leads to the highest-DTM Win state for the opponent.
                        F[p] = "Loss"
                        dtm[p] = max_win_dtm[p] + 1
                        Q.append(p)

    for v in V:
        if F[v] == "Unknown":
            F[v] = "Draw"

    return F, dtm