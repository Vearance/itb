package solver

func Move(board *Board, current *State, dir string) (*State, bool) {
	dx, dy := 0, 0

	switch dir {
		case "U": dx = -1
		case "D": dx = 1
		case "L": dy = -1
		case "R": dy = 1
	}

	next := &State{
		Pos:           current.Pos,
		TotalCost:     current.TotalCost,
		CurrentTarget: current.CurrentTarget,
		Parent:        current,
		Dir:           dir[0],
	}

	curX, curY := current.Pos.X, current.Pos.Y

	for {
		curX += dx
		curY += dy

		// Cek batas papan (Game over kalau lewat)
		if curX < 0 || curX >= board.N || curY < 0 || curY >= board.M {
			return nil, false
		}

		tile := board.Grid[curX][curY]

		// Berhenti tepat sebelum batu 'X'
		if tile == 'X' {
			next.Pos = Point{X: curX - dx, Y: curY - dy}
			return next, true
		}

		// Kena Lava 'L' = Game Over
		if tile == 'L' {
			return nil, false
		}

		// Hitung cost (Tile awal tidak dihitung, tile jalur & berhenti dihitung)
		next.TotalCost += board.Costs[curX][curY]

		// Cek urutan angka 0-9
		if tile >= '0' && tile <= '9' {
			num := int(tile - '0')
			if num == next.CurrentTarget {
				next.CurrentTarget++
			} else if num > next.CurrentTarget {
				// Melanggar urutan = Game Over
				return nil, false
			}
		}
	}
}