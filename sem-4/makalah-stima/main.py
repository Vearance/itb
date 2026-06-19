from graph import generate_graph
from retrograde import retrograde_dynamic_programming
from display import print_optimal_path

def main():
    print("Generating state space graph (V, E)...")
    V, successors, predecessors, terminal_states = generate_graph()
    print(f"Graph built: {len(V)} valid states, {len(terminal_states)} checkmate nodes.")

    print("\nRunning retrograde dynamic programming wave...")
    F, dtm = retrograde_dynamic_programming(V, successors, predecessors, terminal_states)
    print("Tablebase complete.")

    print("\n" + "="*50)
    print("TEST 1: Mate in 1")
    # White King on f6, White Rook on a7, Black King on h8. White to move.
    test_1 = ((5, 5), (0, 6), (7, 7), "White")
    print_optimal_path(test_1, F, dtm, successors)

    print("\n" + "="*50)
    print("TEST 2: Mate in 3 (Random Winning Position)")
    # White King on e5, White Rook on a1, Black King on h8. White to move.
    test_2 = ((4, 4), (0, 0), (7, 7), "White")
    print_optimal_path(test_2, F, dtm, successors)

    print("\n" + "="*50)
    print("TEST 3: Long Mate (Black King in Center, Pieces Spread Out)")
    # White King on a1, White Rook on b8, Black King on e4. White to move.
    # Black king in the center is hardest to corral — requires coordinated play.
    test_3 = ((0, 0), (1, 7), (4, 3), "White")
    print_optimal_path(test_3, F, dtm, successors)

    print("\n" + "="*50)
    print("TEST 4: Draw (Black King Can Capture the Rook)")
    # White King on a1, White Rook on e4, Black King on e5. Black to move.
    # Black king is adjacent to the rook — it can always capture, so the result
    # must be a draw even though white has a rook.
    test_4 = ((0, 0), (4, 3), (4, 4), "Black")
    print_optimal_path(test_4, F, dtm, successors)

    print("\n" + "="*50)
    print("TEST 5: Stalemate Avoidance")
    # White King on g6, White Rook on b7, Black King on h8. White to move.
    # The rook on b7 already covers the entire 7th rank (blocking h7), and
    # the White King on g6 covers g7 — so Black's ONLY escape is g8.
    # The obvious move Rb7-g7?? closes g8 without giving check → STALEMATE.
    # The algorithm must find the correct winning path that avoids this trap.
    test_5 = ((6, 5), (1, 6), (7, 7), "White")
    print_optimal_path(test_5, F, dtm, successors)

if __name__ == "__main__":
    main()