package models

type SelectorResult struct {
	Node *DOMNode
	Path []int
}

type SearchResult struct {
	NodeIDs []int
	Results map[int]SelectorResult
}

type SearchLog struct {
	Selector   Selector
	SearchType string
	Entries    []SearchLogEntry
}

type SearchLogEntry struct {
	NodeID        int
	Depth         int
	CandidateNode bool
	SelectedNode  bool
}

func (res *SearchResult) Serialize() map[int]interface{} {
	serialized := make(map[int]interface{})
	for nodeID, result := range res.Results {
		serialized[nodeID] = map[string]interface{}{
			"node": map[string]interface{}{
				"NodeID":     result.Node.NodeID,
				"Tag":        result.Node.Tag,
				"ID":         result.Node.ID,
				"Classes":    result.Node.Classes,
				"Attributes": result.Node.Attributes,
				"Content":    result.Node.Content,
				"Depth":      result.Node.Depth,
			},
			"path": result.Path,
		}
	}
	return serialized
}

func (log *SearchLog) Serialize() map[string]interface{} {
	entries := make([]interface{}, len(log.Entries))
	for i, entry := range log.Entries {
		entries[i] = map[string]interface{}{
			"NodeID":        entry.NodeID,
			"Depth":         entry.Depth,
			"CandidateNode": entry.CandidateNode,
			"SelectedNode":  entry.SelectedNode,
		}
	}
	return map[string]interface{}{
		"Selector":   log.Selector.String(),
		"SearchType": log.SearchType,
		"Entries":    entries,
	}
}

func (log *SearchLog) UpsertLogEntry(entry SearchLogEntry) {
	if log == nil {
		return
	}

	for i := range log.Entries {
		if log.Entries[i].NodeID != entry.NodeID {
			continue
		}

		if entry.CandidateNode {
			log.Entries[i].CandidateNode = true
		}
		if entry.SelectedNode {
			log.Entries[i].SelectedNode = true
		}

		return
	}

	log.Entries = append(log.Entries, entry)
}
