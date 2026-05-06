package solver

import "math"

// Heuristic 1: Manhattan Distance
// h(n) = |xi - xp| + |yi - yp|
func ManhattanDistance(pos Point, board *Board, target int) int {
	var targetPoint Point
	found := false

	// jika target masih ada di papan (kurang dari atau sama dengan angka tertinggi)
	if target <= board.MaxTarget && board.MaxTarget != -1 {
		targetChar := rune(target + '0')
		for i := 0; i < board.N; i++ {
			for j := 0; j < board.M; j++ {
				if board.Grid[i][j] == targetChar {
					targetPoint = Point{i, j}
					found = true
					break
				}
			}
			if found {
				break
			}
		}
	} 
	
	// jika semua angka sudah diambil, atau memang tidak ada angka di papan
	if !found {
		for i := 0; i < board.N; i++ {
			for j := 0; j < board.M; j++ {
				if board.Grid[i][j] == 'O' {
					targetPoint = Point{i, j}
					found = true
					break
				}
			}
			if found {
				break
			}
		}
	}

	return int(math.Abs(float64(pos.X-targetPoint.X)) + math.Abs(float64(pos.Y-targetPoint.Y)))
}

// Heuristic 2: Euclidean Distance
// h(n) = sqrt((xi - xp)^2 + (yi - yp)^2)
func EuclideanDistance(pos Point, board *Board, target int) int {
    var targetPoint Point
    found := false

	// jika target masih ada di papan (kurang dari atau sama dengan angka tertinggi)
    if target <= board.MaxTarget && board.MaxTarget != -1 {
        targetChar := rune(target + '0')
        for i := 0; i < board.N; i++ {
            for j := 0; j < board.M; j++ {
                if board.Grid[i][j] == targetChar {
                    targetPoint = Point{i, j}
                    found = true
                    break
                }
            }
            if found {
                break
            }
        }
    } 
    
    // jika semua angka sudah diambil, atau memang tidak ada angka di papan
    if !found {
        for i := 0; i < board.N; i++ {
            for j := 0; j < board.M; j++ {
                if board.Grid[i][j] == 'O' {
                    targetPoint = Point{i, j}
                    found = true
                    break
                }
            }
            if found {
                break
            }
        }
    }

    dx := float64(pos.X - targetPoint.X)
    dy := float64(pos.Y - targetPoint.Y)
    
    return int(math.Sqrt(dx*dx + dy*dy))
}

// Heuristic 3: Chebyshev Distance
// h(n) = max(|xi - xp|, |yi - yp|)
func ChebyshevDistance(pos Point, board *Board, target int) int {
	var targetPoint Point
	found := false

	// jika target masih ada di papan (kurang dari atau sama dengan angka tertinggi)
	if target <= board.MaxTarget && board.MaxTarget != -1 {
		targetChar := rune(target + '0')
		for i := 0; i < board.N; i++ {
			for j := 0; j < board.M; j++ {
				if board.Grid[i][j] == targetChar {
					targetPoint = Point{i, j}
					found = true
					break
				}
			}
			if found {
				break
			}
		}
	} 
	
	// jika semua angka sudah diambil, atau memang tidak ada angka di papan
	if !found {
		for i := 0; i < board.N; i++ {
			for j := 0; j < board.M; j++ {
				if board.Grid[i][j] == 'O' {
					targetPoint = Point{i, j}
					found = true
					break
				}
			}
			if found {
				break
			}
		}
	}

	dx := math.Abs(float64(pos.X - targetPoint.X))
	dy := math.Abs(float64(pos.Y - targetPoint.Y))
	return int(math.Max(dx, dy))
}