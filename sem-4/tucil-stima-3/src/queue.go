package solver

type PriorityQueue []*Item

func (pq *PriorityQueue) Len() int { return len(*pq) }

func (pq *PriorityQueue) Push(it *Item) {
	*pq = append(*pq, it)
	pq.siftUp(len(*pq) - 1)
}

func (pq *PriorityQueue) Pop() *Item {
	old := *pq
	n := len(old)
	top := old[0]
	old[0] = old[n-1]
	old[n-1] = nil
	*pq = old[0 : n-1]
	if len(*pq) > 0 {
		pq.siftDown(0)
	}
	return top
}

func (pq *PriorityQueue) siftUp(i int) {
	for i > 0 {
		parent := (i - 1) / 2
		if (*pq)[i].priority < (*pq)[parent].priority {
			(*pq)[i], (*pq)[parent] = (*pq)[parent], (*pq)[i]
			i = parent
		} else {
			break
		}
	}
}

func (pq *PriorityQueue) siftDown(i int) {
	n := len(*pq)
	for {
		l := 2*i + 1
		r := 2*i + 2
		smallest := i
		if l < n && (*pq)[l].priority < (*pq)[smallest].priority {
			smallest = l
		}
		if r < n && (*pq)[r].priority < (*pq)[smallest].priority {
			smallest = r
		}
		if smallest == i {
			break
		}
		(*pq)[i], (*pq)[smallest] = (*pq)[smallest], (*pq)[i]
		i = smallest
	}
}