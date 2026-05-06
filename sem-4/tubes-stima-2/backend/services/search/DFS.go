package search

import (
	"backend/models"
	"sync"
	"sync/atomic"
)

func searchDFS(node *models.DOMNode, selector *models.Selector, stepIndex int, amount int, results *models.SearchResult, log *models.SearchLog, mu *sync.Mutex, stop *atomic.Bool) {
	if len(selector.Steps) == 0 || stepIndex < 0 || stepIndex >= len(selector.Steps) {
		return
	}
	if stop.Load() {
		return
	}

	if stepIndex == 0 {
		traverse(node, func(node *models.DOMNode) bool {
			if stop.Load() {
				return false
			}

			matchElement(node, selector, 0, amount, results, log, mu, stop)
			return !stop.Load()
		}, stop)
		return
	}

	matchElement(node, selector, stepIndex, amount, results, log, mu, stop)
}

func SearchElementDFS(root *models.DOMNode, selector *models.Selector, amount int) (*models.SearchResult, *models.SearchLog) {
	if root == nil || selector == nil {
		return nil, nil
	}

	if amount <= 0 {
		amount = int(^uint(0) >> 1)
	}

	results := models.SearchResult{
		NodeIDs: []int{},
		Results: make(map[int]models.SelectorResult),
	}

	log := models.SearchLog{
		Selector:   *selector,
		SearchType: "DFS",
		Entries:    []models.SearchLogEntry{},
	}

	var mu sync.Mutex
	var stop atomic.Bool

	searchDFS(root, selector, 0, amount, &results, &log, &mu, &stop)
	return &results, &log
}

func matchElement(node *models.DOMNode, selector *models.Selector, stepIndex int, amount int, results *models.SearchResult, log *models.SearchLog, mu *sync.Mutex, stop *atomic.Bool) {
	if node == nil || selector == nil || results == nil {
		return
	}
	if stepIndex < 0 || stepIndex >= len(selector.Steps) {
		return
	}
	if stop.Load() {
		return
	}

	isCandidate := selector.Steps[stepIndex].Compound.Matches(node)

	mu.Lock()
	log.UpsertLogEntry(models.SearchLogEntry{
		NodeID:        node.NodeID,
		Depth:         node.Depth,
		CandidateNode: isCandidate,
		SelectedNode:  isCandidate && stepIndex == len(selector.Steps)-1,
	})
	mu.Unlock()

	step := selector.Steps[stepIndex]
	if !step.Compound.Matches(node) {
		return
	}

	if stepIndex == len(selector.Steps)-1 {
		path := make([]int, 0, node.Depth+1)
		for current := node; current != nil; current = current.Parent {
			path = append([]int{current.NodeID}, path...)
		}

		mu.Lock()
		if stop.Load() {
			mu.Unlock()
			return
		}
		if _, exists := (*results).Results[node.NodeID]; !exists {
			(*results).Results[node.NodeID] = models.SelectorResult{
				Node: node,
				Path: path,
			}

			(*results).NodeIDs = append((*results).NodeIDs, node.NodeID)
			
			if len((*results).NodeIDs) >= amount {
				stop.Store(true)
			}
		}
		mu.Unlock()

		return
	}

	nextStep := selector.Steps[stepIndex+1]
	for _, candidate := range node.GetRelatedNodes(nextStep.Combinator) {
		if stop.Load() {
			return
		}

		matchElement(candidate, selector, stepIndex+1, amount, results, log, mu, stop)
	}
}

func traverse(node *models.DOMNode, visit func(*models.DOMNode) bool, stop *atomic.Bool) {
	if node == nil || visit == nil {
		return
	}
	
	if stop.Load() {
		return
	}

	if !visit(node) {
		return
	}

	var wg sync.WaitGroup
	for _, child := range node.Children {
		wg.Add(1)
		go func(c *models.DOMNode) {
			defer wg.Done()
			traverse(c, visit, stop)
		}(child)
	}
	wg.Wait()
}
