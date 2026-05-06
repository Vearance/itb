package solver

type Point struct {
	X, Y int
}

type Board struct {
	N, M  int
	Grid  [][]rune
	Costs [][]int
	MaxTarget int
}

type State struct {
	Pos           Point
	TotalCost     int
	CurrentTarget int  // untuk urutan angka 0-9
	Parent        *State
	Dir           byte
}

// used in priority queue
type Item struct {
	state    *State
	priority int
	index    int
}

type key struct {
	X, Y, T int
}