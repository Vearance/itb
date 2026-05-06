package search

import (
	"backend/models"
	"sort"
)

type QueueItem struct {
	Node      *models.DOMNode
	StepIndex int
	Order     int
}

func SearchElementBFS(root *models.DOMNode, selector *models.Selector, amount int) (*models.SearchResult, *models.SearchLog) {
	if root == nil || selector == nil {
		return nil, nil
	}

	results := models.SearchResult{
		NodeIDs: []int{},
		Results: make(map[int]models.SelectorResult),
	}

	log := models.SearchLog{
		Selector:   *selector,
		SearchType: "BFS",
		Entries:    []models.SearchLogEntry{},
	}

	queue := []QueueItem{}
	enqueueOrder := 0
	enqueue := func(node *models.DOMNode, stepIndex int) {
		queue = append(queue, QueueItem{Node: node, StepIndex: stepIndex, Order: enqueueOrder})
		enqueueOrder++
	}

	enqueue(root, 0)

	if amount <= 0 {
		amount = int(^uint(0) >> 1)
	}

	for len(queue) > 0 && len(results.NodeIDs) < amount {
		sort.SliceStable(queue, func(i, j int) bool {
			leftItem := queue[i]
			rightItem := queue[j]

			l := int(^uint(0) >> 1)
			r := int(^uint(0) >> 1)

			if leftItem.Node != nil {
				l = leftItem.Node.Depth
			}
			if rightItem.Node != nil {
				r = rightItem.Node.Depth
			}

			if l != r {
				return l < r
			}

			return leftItem.Order < rightItem.Order
		})

		currentItem := queue[0]
		queue = queue[1:]

		currentNode := currentItem.Node
		currentStepIndex := currentItem.StepIndex

		if currentNode == nil || currentStepIndex < 0 || currentStepIndex >= len(selector.Steps) {
			continue
		}

		logEntry := models.SearchLogEntry{
			NodeID:        currentNode.NodeID,
			Depth:         currentNode.Depth,
			CandidateNode: selector.Steps[currentStepIndex].Compound.Matches(currentNode),
			SelectedNode:  selector.Steps[currentStepIndex].Compound.Matches(currentNode) && currentStepIndex == len(selector.Steps)-1,
		}

		log.UpsertLogEntry(logEntry)

		step := selector.Steps[currentStepIndex]

		for _, child := range currentNode.Children {
			enqueue(child, currentStepIndex)
		}

		if step.Compound.Matches(currentNode) {
			if currentStepIndex == len(selector.Steps)-1 {
				path := make([]int, 0, currentNode.Depth+1)
				for current := currentNode; current != nil; current = current.Parent {
					path = append([]int{current.NodeID}, path...)
				}

				if _, exists := results.Results[currentNode.NodeID]; !exists {
					results.Results[currentNode.NodeID] = models.SelectorResult{
						Node: currentNode,
						Path: path,
					}

					results.NodeIDs = append(results.NodeIDs, currentNode.NodeID)
				}
			} else {
				nextCombinator := selector.Steps[currentStepIndex+1].Combinator
				relatedNodes := currentNode.GetRelatedNodes(nextCombinator)

				if nextCombinator == models.CombinatorDescendant || nextCombinator == models.CombinatorNone {
					sort.SliceStable(relatedNodes, func(i, j int) bool {
						if relatedNodes[i].Depth != relatedNodes[j].Depth {
							return relatedNodes[i].Depth < relatedNodes[j].Depth
						}
						return relatedNodes[i].NodeID < relatedNodes[j].NodeID
					})
				}

				for _, relatedNode := range relatedNodes {
					if len(results.NodeIDs) >= amount {
						break
					}

					enqueue(relatedNode, currentStepIndex+1)
				}
			}
		}
	}

	return &results, &log
}
