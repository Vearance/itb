package utils

import (
	"backend/models"
	"fmt"
	"strings"
)

func PrintDOMTree(node *models.DOMNode, prefix string, isLast bool) {
	connector := "├── "
	if isLast {
		connector = "└── "
	}

	var label strings.Builder
	label.WriteString("<" + node.Tag + ">" + fmt.Sprintf(" (NodeID: %d)", node.NodeID))

	if node.Content != "" {
		label.WriteString(" Content: '" + node.Content + "'")
	}

	for key, value := range node.Attributes {
		label.WriteString(fmt.Sprintf(" [%s=%q]", key, value))
	}

	fmt.Println(prefix + connector + label.String())

	childPrefix := prefix + "│   "
	if isLast {
		childPrefix = prefix + "    "
	}

	for i, child := range node.Children {
		PrintDOMTree(child, childPrefix, i == len(node.Children)-1)
	}
}

func PrintTree(root *models.DOMNode) {
	fmt.Printf("<%s> (NodeID: %d)\n", root.Tag, root.NodeID)
	for i, child := range root.Children {
		PrintDOMTree(child, "", i == len(root.Children)-1)
	}
}
