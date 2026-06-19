from rules import is_valid_square, get_king_moves, get_rook_moves, is_attacked_by_rook

def is_legal_state(wk, wr, bk, turn):
    # Pieces cannot overlap
    if wk == wr or wk == bk or wr == bk: return False
    # Kings cannot touch
    if max(abs(wk[0]-bk[0]), abs(wk[1]-bk[1])) <= 1: return False
    # If it is White's turn, Black just moved. Black cannot leave themselves in check.
    if turn == "White" and is_attacked_by_rook(bk, wr, wk): return False
    return True

def generate_graph():
    V = []
    for wk_x in range(8):
        for wk_y in range(8):
            for wr_x in range(8):
                for wr_y in range(8):
                    for bk_x in range(8):
                        for bk_y in range(8):
                            for turn in ["White", "Black"]:
                                wk, wr, bk = (wk_x, wk_y), (wr_x, wr_y), (bk_x, bk_y)
                                if is_legal_state(wk, wr, bk, turn):
                                    V.append((wk, wr, bk, turn))

    successors = {v: [] for v in V}
    predecessors = {v: [] for v in V}
    V_set = set(V)

    for state in V:
        wk, wr, bk, turn = state
        if turn == "White":
            for move in get_king_moves(wk):
                next_state = (move, wr, bk, "Black")
                if next_state in V_set:
                    successors[state].append(next_state)
                    predecessors[next_state].append(state)
            for move in get_rook_moves(wr, [wk, bk]):
                next_state = (wk, move, bk, "Black")
                if next_state in V_set:
                    successors[state].append(next_state)
                    predecessors[next_state].append(state)
        else:
            for move in get_king_moves(bk):
                if move == wr:
                    # BK tries to capture the White Rook.
                    # This is only legal if the White King does NOT protect the rook's square
                    # (i.e. WK is not adjacent to WR). If WK is adjacent to WR, BK would be
                    # stepping into check from WK, which is illegal.
                    if max(abs(wk[0] - wr[0]), abs(wk[1] - wr[1])) > 1:
                        successors[state].append("DRAW_STATE")
                    continue
                next_state = (wk, wr, move, "White")
                # Black cannot step into check
                if next_state in V_set and not is_attacked_by_rook(move, wr, wk):
                    successors[state].append(next_state)
                    predecessors[next_state].append(state)

    terminal_states = []
    for state in V:
        wk, wr, bk, turn = state
        if turn == "Black":
            # Checkmate occurs if Black is in check and has no legal moves
            if is_attacked_by_rook(bk, wr, wk) and len(successors[state]) == 0:
                terminal_states.append(state)

    return V, successors, predecessors, terminal_states