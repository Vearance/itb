package main

//exhaustive solution
func Solve(b *Board, counter *int) bool {
	rowPlaced := make([]int, b.size)

	for {
		(*counter)++

		// clear all queens
		for i := 0; i < b.size; i++ {
			for j := 0; j < b.size; j++ {
				if b.queen[i][j] {
					b.rmvQueen(i, j)
				}
			}
		}

		valid := true
		for row := 0; row < b.size; row++ {
			col := rowPlaced[row]
			if b.isPlaceable(row, col) {
				b.addQueen(row, col)
			} else {
				valid = false
				break
			}
		}

		liveBoard(b, *counter)

		//if all queens were successfully placed, we found a solution
		if valid {
			return true
		}

		carry := true
		for row := b.size - 1; row >= 0 && carry; row-- {
			rowPlaced[row]++
			if rowPlaced[row] < b.size {
				carry = false
			} else {
				rowPlaced[row] = 0
			}
		}

		// If carry is still true, we've exhausted all n^n combinations
		if carry {
			break
		}
	}

	// no solution found
	for i := 0; i < b.size; i++ {
		for j := 0; j < b.size; j++ {
			if b.queen[i][j] {
				b.rmvQueen(i, j)
			}
		}
	}
	return false
}

func SolveEffective(b *Board, counter *int) bool {
	row := 0;
	rowPlaced := make([]int, b.size)

	for i:=0; i < len(rowPlaced); i++ {
		rowPlaced[i] = -1  // queen does not exist; later save column
	}

	for row >= 0 {  // while syntax in Go
		var placedOnThisRow bool
		startCol := 0
		
		// start after the row that was implemented
		if rowPlaced[row] != -1 { // implemented
			startCol = rowPlaced[row] + 1
			b.rmvQueen(row, rowPlaced[row])
			liveBoard(b, *counter)
		}
		
		for i:=startCol; i < b.size; i++ {
			(*counter)++

			if b.isPlaceable(row, i) {
				b.addQueen(row, i)
				rowPlaced[row] = i  // save Queen's column number on that row 

				liveBoard(b, *counter)

				row = row + 1
				placedOnThisRow = true

				break
			}

		}

		if !placedOnThisRow {
			rowPlaced[row] = -1  // say the queen does not exist again
			row = row - 1
		}

		if row == b.size {
			return true
		}

	}

	return false
}