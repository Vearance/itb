def is_valid_square(sq):
    return 0 <= sq[0] <= 7 and 0 <= sq[1] <= 7

def get_king_moves(sq):
    moves = []
    for dx in [-1, 0, 1]:
        for dy in [-1, 0, 1]:
            if dx == 0 and dy == 0:
                continue
            new_sq = (sq[0] + dx, sq[1] + dy)
            if is_valid_square(new_sq):
                moves.append(new_sq)
    return moves

def get_rook_moves(sq, blockers):
    moves = []
    directions = [(0, 1), (1, 0), (0, -1), (-1, 0)]
    for dx, dy in directions:
        for i in range(1, 8):
            new_sq = (sq[0] + dx * i, sq[1] + dy * i)
            if not is_valid_square(new_sq):
                break
            moves.append(new_sq)
            if new_sq in blockers:
                break
    return moves

def is_attacked_by_rook(target_sq, rook_sq, king_sq):
    if target_sq[0] == rook_sq[0]:
        step = 1 if target_sq[1] > rook_sq[1] else -1
        for y in range(rook_sq[1] + step, target_sq[1], step):
            if (rook_sq[0], y) == king_sq:
                return False
        return True
    if target_sq[1] == rook_sq[1]:
        step = 1 if target_sq[0] > rook_sq[0] else -1
        for x in range(rook_sq[0] + step, target_sq[0], step):
            if (x, rook_sq[1]) == king_sq:
                return False
        return True
    return False

def sq_to_algebraic(sq):
    files = "abcdefgh"
    return f"{files[sq[0]]}{sq[1] + 1}"