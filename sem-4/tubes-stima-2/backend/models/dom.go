package models

type DOMNode struct {
	NodeID     int
	Tag        string
	ID         string
	Classes    []string
	Attributes map[string]string
	Parent     *DOMNode
	Children   []*DOMNode
	Content    string
	Depth      int
}

func (n *DOMNode) Flatten() []*DOMNode {
	var nodes []*DOMNode
	var flatten func(node *DOMNode)
	flatten = func(node *DOMNode) {
		if node == nil {
			return
		}
		nodes = append(nodes, node)
		for _, child := range node.Children {
			flatten(child)
		}
	}
	flatten(n)
	return nodes
}

func (n *DOMNode) FlattenChildren() []*DOMNode {
	var nodes []*DOMNode
	for _, child := range n.Children {
		nodes = append(nodes, child.Flatten()...)
	}
	return nodes
}

func (node *DOMNode) GetRelatedNodes(combinator Combinator) []*DOMNode {
	switch combinator {
	case CombinatorChild:
		return append([]*DOMNode(nil), node.Children...)
	case CombinatorAdjacentSibling:
		if node.Parent == nil {
			return nil
		}

		siblings := node.Parent.Children
		for i, sibling := range siblings {
			if sibling != node {
				continue
			}
			if i+1 < len(siblings) {
				return []*DOMNode{siblings[i+1]}
			}
			return nil
		}
		return nil
	case CombinatorGeneralSibling:
		if node.Parent == nil {
			return nil
		}

		siblings := node.Parent.Children
		for i, sibling := range siblings {
			if sibling != node {
				continue
			}
			if i+1 < len(siblings) {
				return append([]*DOMNode(nil), siblings[i+1:]...)
			}
			return nil
		}
		return nil
	case CombinatorDescendant, CombinatorNone:
		fallthrough
	default:
		return node.FlattenChildren()
	}
}

func (node *DOMNode) Serialize() map[string]interface{} {
	serialized := map[string]interface{}{
		"nodeID":     node.NodeID,
		"tag":        node.Tag,
		"id":         node.ID,
		"classes":    node.Classes,
		"attributes": node.Attributes,
		"content":    node.Content,
		"depth":      node.Depth,
	}

	if len(node.Children) > 0 {
		children := make([]map[string]interface{}, len(node.Children))
		for i, child := range node.Children {
			children[i] = child.Serialize()
		}
		serialized["children"] = children
	} else {
		serialized["children"] = []map[string]interface{}{}
	}

	return serialized
}
