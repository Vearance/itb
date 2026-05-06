package solver

import "math"

func isGoal(s *State, b *Board) bool {
	if b.Grid[s.Pos.X][s.Pos.Y] != 'O' {
		return false
	}
	if b.MaxTarget == -1 {
		return true
	}
	return s.CurrentTarget > b.MaxTarget
}

func Reverse(goal *State) (string, []Point) {
	n := 0
	for s := goal; s != nil; s = s.Parent {
		n++
	}
	ps := make([]Point, n)
	dirs := make([]byte, n-1)
	i := n - 1
	for s := goal; s != nil; s = s.Parent {
		ps[i] = s.Pos
		if s.Parent != nil {
			dirs[i-1] = s.Dir
		}
		i--
	}
	return string(dirs), ps
}

func UCS(board *Board, startPos Point) (*State, int) {
	pq := &PriorityQueue{}

	visited := make(map[key]int)
	dirs := []string{"U", "D", "L", "R"}

	first := &State{Pos: startPos, TotalCost: 0, CurrentTarget: 0}
	pq.Push(&Item{state: first, priority: 0})

	iter := 0
	for pq.Len() > 0 {
		it := pq.Pop()
		cur := it.state
		iter++

		if isGoal(cur, board) {
			return cur, iter
		}

		k := key{cur.Pos.X, cur.Pos.Y, cur.CurrentTarget}
		prev, ok := visited[k]
		if ok && prev <= cur.TotalCost {
			continue
		}
		visited[k] = cur.TotalCost

		for i := 0; i < len(dirs); i++ {
			next, ok := Move(board, cur, dirs[i])
			if !ok {
				continue
			}
			nk := key{next.Pos.X, next.Pos.Y, next.CurrentTarget}
			pv, vok := visited[nk]
			if vok && pv <= next.TotalCost {
				continue
			}
			pq.Push(&Item{state: next, priority: next.TotalCost})
		}
	}

	return nil, iter
}

func GBFS(board *Board, startPos Point, h func(Point, *Board, int) int) (*State, int) {
	pq := &PriorityQueue{}

	visited := make(map[key]bool)
	dirs := []string{"U", "D", "L", "R"}

	first := &State{Pos: startPos, TotalCost: 0, CurrentTarget: 0}
	pq.Push(&Item{state: first, priority: h(startPos, board, 0)})

	iter := 0
	for pq.Len() > 0 {
		it := pq.Pop()
		cur := it.state
		iter++

		if isGoal(cur, board) {
			return cur, iter
		}

		k := key{cur.Pos.X, cur.Pos.Y, cur.CurrentTarget}
		if visited[k] {
			continue
		}
		visited[k] = true

		for i := 0; i < len(dirs); i++ {
			next, ok := Move(board, cur, dirs[i])
			if !ok {
				continue
			}
			if visited[key{next.Pos.X, next.Pos.Y, next.CurrentTarget}] {
				continue
			}
			hv := h(next.Pos, board, next.CurrentTarget)
			pq.Push(&Item{state: next, priority: hv})
		}
	}

	return nil, iter
}

func AStar(board *Board, startPos Point, h func(Point, *Board, int) int) (*State, int) {
	pq := &PriorityQueue{}

	visited := make(map[key]int)
	dirs := []string{"U", "D", "L", "R"}

	first := &State{Pos: startPos, TotalCost: 0, CurrentTarget: 0}
	pq.Push(&Item{state: first, priority: h(startPos, board, 0)})

	iter := 0
	for pq.Len() > 0 {
		it := pq.Pop()
		cur := it.state
		iter++

		if isGoal(cur, board) {
			return cur, iter
		}

		k := key{cur.Pos.X, cur.Pos.Y, cur.CurrentTarget}
		prev, ok := visited[k]
		if ok && prev <= cur.TotalCost {
			continue
		}
		visited[k] = cur.TotalCost

		for i := 0; i < len(dirs); i++ {
			next, ok := Move(board, cur, dirs[i])
			if !ok {
				continue
			}
			nk := key{next.Pos.X, next.Pos.Y, next.CurrentTarget}
			pv, vok := visited[nk]
			if vok && pv <= next.TotalCost {
				continue
			}
			hv := h(next.Pos, board, next.CurrentTarget)
			f := next.TotalCost + hv
			pq.Push(&Item{state: next, priority: f})
		}
	}

	return nil, iter
}

func BFS(board *Board, startPos Point) (*State, int) {
	visited := make(map[key]bool)
	dirs := []string{"U", "D", "L", "R"}

	first := &State{Pos: startPos, TotalCost: 0, CurrentTarget: 0}
	queue := []*State{first}
	visited[key{startPos.X, startPos.Y, 0}] = true

	iter := 0
	for len(queue) > 0 {
		cur := queue[0]
		queue = queue[1:]
		iter++

		if isGoal(cur, board) {
			return cur, iter
		}

		for i := 0; i < len(dirs); i++ {
			next, ok := Move(board, cur, dirs[i])
			if !ok {
				continue
			}
			k := key{next.Pos.X, next.Pos.Y, next.CurrentTarget}
			if visited[k] {
				continue
			}
			visited[k] = true
			queue = append(queue, next)
		}
	}

	return nil, iter
}

func IDAStar(board *Board, startPos Point, h func(Point, *Board, int) int) (*State, int) {
	first := &State{Pos: startPos, TotalCost: 0, CurrentTarget: 0}
	iter := 0
	threshold := h(startPos, board, 0)

	for {
		nextLimit, goal := idaSearch(board, first, threshold, h, &iter)
		if goal != nil {
			return goal, iter
		}
		if nextLimit == math.MaxInt {
			return nil, iter
		}
		threshold = nextLimit
	}
}

func idaSearch(board *Board, cur *State, threshold int, h func(Point, *Board, int) int, iter *int) (int, *State) {
	*iter++
	hv := h(cur.Pos, board, cur.CurrentTarget)
	f := cur.TotalCost + hv
	if f > threshold {
		return f, nil
	}
	if isGoal(cur, board) {
		return f, cur
	}

	min := math.MaxInt
	dirs := []string{"U", "D", "L", "R"}
	for i := 0; i < len(dirs); i++ {
		next, ok := Move(board, cur, dirs[i])
		if !ok {
			continue
		}
		if isOnPath(cur, next.Pos, next.CurrentTarget) {
			continue
		}
		nextLimit, goal := idaSearch(board, next, threshold, h, iter)
		if goal != nil {
			return nextLimit, goal
		}
		if nextLimit < min {
			min = nextLimit
		}
	}
	return min, nil
}

func isOnPath(s *State, pos Point, target int) bool {
	for c := s; c != nil; c = c.Parent {
		if c.Pos == pos && c.CurrentTarget == target {
			return true
		}
	}
	return false
}
