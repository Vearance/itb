package main

type Board struct {
    size int
    color [][]rune
	queen [][]bool
}

func newBoard(colorMatrix [][]rune) *Board {
	size := len(colorMatrix)

	queenMatrix := make([][]bool, size)
	for i := 0; i < size; i++ {
		queenMatrix[i] = make([]bool, size)
	}

	return &Board {
		size: size,
		color: colorMatrix,
		queen: queenMatrix,
	}
}

func (b *Board) isValid() bool {  // every color must appear once, number of color == board size/length
	// if empty return false
	if b.size == 0 {
		return false
	}

	// if not square return false
	if len(b.color) != b.size {
		return false
	}

	for k:=0; k < b.size; k++ {
		if len(b.color[k]) != b.size {
			return false
		}
	}

	// check unique colors
	unique := make(map[rune]bool)
	for i:=0; i < b.size; i++ {
		for j:=0; j < b.size; j++ {
			unique[b.color[i][j]] = true
		}
	}
	count := len(unique)

	if count == b.size {
		return true
	}

	return true
}

func (b *Board) isPlaceable(row, col int) bool {
	if b.queen[row][col] == true {
		return false
	}

	// one per row
	for i:=0; i < b.size; i++ {
		if b.queen[row][i] == true {
			return false
		}
	}

	// one per col
	for j:=0; j < b.size; j++ {
		if b.queen[j][col] == true {
			return false
		}
	}

	// cant be right beside each other, including diagonal
	sides := [8][2]int {
		{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1},
	}

	for _, val := range sides {
		// validate index
		newRow := row + val[0]
		newCol := col + val[1]

		if newRow >= 0 && newCol >= 0 && newRow < b.size && newCol < b.size {
			if b.queen[newRow][newCol] == true {
				return false
			}
		}
	}

	// no two queens can be on the same color
	currentColor := b.color[row][col]
	for i:=0; i < b.size; i++ {
		for j:=0; j < b.size; j++ {
			if b.queen[i][j] == true && b.color[i][j] == currentColor {
				return false
			}
		}
	}

	return true
}

func (b *Board) addQueen(row, col int) {
	b.queen[row][col] = true
}

func (b *Board) rmvQueen(row, col int) {
	b.queen[row][col] = false
}