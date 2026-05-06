package parser

import (
	"backend/models"
	"io"
	"strings"

	"golang.org/x/net/html"
)

func ParseHTML(rawHTML string) (*models.DOMNode, error) {
	z := html.NewTokenizer(strings.NewReader(rawHTML))

	var root *models.DOMNode
	var current *models.DOMNode
	nodeID := 1

	for {
		tt := z.Next()
		switch tt {
		case html.ErrorToken:
			err := z.Err()
			if err == nil || err == io.EOF {
				return root, nil
			}
			return nil, err
		case html.StartTagToken, html.SelfClosingTagToken:
			t := z.Token()
			node := &models.DOMNode{
				NodeID:     nodeID,
				Tag:        t.Data,
				ID:         "",
				Classes:    []string{},
				Attributes: make(map[string]string),
				Parent:     current,
				Children:   []*models.DOMNode{},
				Content:    "",
				Depth:      0,
			}
			nodeID++

			for _, attr := range t.Attr {
				switch attr.Key {
				case "id":
					node.ID = attr.Val
				case "class":
					node.Classes = append(node.Classes, strings.Split(attr.Val, " ")...)
				}
				node.Attributes[attr.Key] = attr.Val
			}

			if current != nil {
				current.Children = append(current.Children, node)
				node.Depth = current.Depth + 1
			} else {
				root = node
			}
			if tt == html.StartTagToken {
				current = node
			}
		case html.EndTagToken:
			if current != nil {
				current = current.Parent
			}
		case html.TextToken:
			if current != nil {
				text := strings.TrimSpace(string(z.Text()))
				current.Content += text
				if text != "" {
					current.Content = strings.Join([]string{current.Content, text}, "")
				}
			}
		}
	}
}
