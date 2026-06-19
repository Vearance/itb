from rules import sq_to_algebraic

def state_to_fen(state):
    wk, wr, bk, turn = state
    board = [["" for _ in range(8)] for _ in range(8)]
    board[wk[1]][wk[0]] = "K"
    board[wr[1]][wr[0]] = "R"
    board[bk[1]][bk[0]] = "k"

    fen_rows = []
    for y in range(7, -1, -1):
        empty = 0
        row_str = ""
        for x in range(8):
            piece = board[y][x]
            if piece == "":
                empty += 1
            else:
                if empty > 0:
                    row_str += str(empty)
                    empty = 0
                row_str += piece
        if empty > 0:
            row_str += str(empty)
        fen_rows.append(row_str)

    fen = "/".join(fen_rows)
    active_color = "w" if turn == "White" else "b"
    return f"{fen} {active_color} - - 0 1"

def print_optimal_path(start_state, F, dtm, successors):
    if start_state not in F:
        print("Invalid state (Not in graph).")
        return

    if F[start_state] == "Draw":
        print(f"FEN: {state_to_fen(start_state)} | Result: Forced Draw")
        return

    current_state = start_state
    print(f"\nEvaluating FEN: {state_to_fen(current_state)}")
    print(f"DP Result: {F[current_state]} in {dtm[current_state]} plies (half-moves)")
    print("-" * 50)

    while dtm[current_state] > 0:
        turn = current_state[3]
        best_next = None

        # Bellman optimality: find the successor that reduces DTM by exactly 1.
        # White plays toward a Loss state; Black plays toward a Win state.
        for succ in successors[current_state]:
            if succ == "DRAW_STATE": continue
            if turn == "White" and F[succ] == "Loss" and dtm[succ] == dtm[current_state] - 1:
                best_next = succ
                break
            elif turn == "Black" and F[succ] == "Win" and dtm[succ] == dtm[current_state] - 1:
                best_next = succ
                break

        if best_next is None:
            print("Error: could not find optimal next move (DTM inconsistency).")
            break

        # Calculate move notation
        move_str = ""
        if current_state[0] != best_next[0]:
            move_str = f"K{sq_to_algebraic(current_state[0])}-{sq_to_algebraic(best_next[0])}"
        elif current_state[1] != best_next[1]:
            move_str = f"R{sq_to_algebraic(current_state[1])}-{sq_to_algebraic(best_next[1])}"
        elif current_state[2] != best_next[2]:
            move_str = f"k{sq_to_algebraic(current_state[2])}-{sq_to_algebraic(best_next[2])}"

        print(f"[DTM {dtm[current_state]:02d}] {state_to_fen(current_state)}")
        print(f"         --> {turn} plays {move_str}")

        current_state = best_next

    print(f"[DTM 00] {state_to_fen(current_state)} (CHECKMATE)")
