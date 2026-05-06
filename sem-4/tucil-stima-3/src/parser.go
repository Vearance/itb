package solver

import (
	"bufio"
	"os"
	"strconv"
	"strings"
)

func LoadMap(filename string) (*Board, Point, error) {
	file, err := os.Open(filename)
	if err != nil {
		return nil, Point{}, err
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	scanner.Scan()
	dims := strings.Fields(scanner.Text())
	n, _ := strconv.Atoi(dims[0])
	m, _ := strconv.Atoi(dims[1])

	board := &Board{
		N: n,
		M: m,
		Grid: make([][]rune, n),
		Costs: make([][]int, n),
		MaxTarget: -1,
	}

	var startPos Point
	// read grid
	maxNum := -1
	for i := 0; i < n; i++ {
		scanner.Scan()
		line := scanner.Text()
		board.Grid[i] = []rune(line)
		for j, char := range board.Grid[i] {
			// check maxTarget
			if char >= '0' && char <= '9' {
				num := int(char - '0')
				if num > maxNum {
					maxNum = num
				}
			}
			if char == 'Z' {  // posisi awal
				startPos = Point{ X: i, Y: j }
			}
		}
	}
	board.MaxTarget = maxNum

	// read cost for each point
	for i := 0; i < n; i++ {
		scanner.Scan()
		costs := strings.Fields(scanner.Text())
		board.Costs[i] = make([]int, m)
		for j, c := range costs {
			val, _ := strconv.Atoi(c)
			board.Costs[i][j] = val
		}
	}

	return board, startPos, nil
}