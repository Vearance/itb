package models

import (
	"fmt"
	"slices"
	"strings"
)

type Combinator string

const (
	CombinatorNone            Combinator = ""
	CombinatorDescendant      Combinator = " "
	CombinatorChild           Combinator = ">"
	CombinatorAdjacentSibling Combinator = "+"
	CombinatorGeneralSibling  Combinator = "~"
)

type AttrOperator string

const (
	AttrOperatorExists         AttrOperator = "exists"
	AttrOperatorEquals         AttrOperator = "="
	AttrOperatorIncludes       AttrOperator = "~="
	AttrOperatorDashMatch      AttrOperator = "|="
	AttrOperatorPrefixMatch    AttrOperator = "^="
	AttrOperatorSuffixMatch    AttrOperator = "$="
	AttrOperatorSubstringMatch AttrOperator = "*="
)

type AttributeSelector struct {
	Name     string
	Operator AttrOperator
	Value    string
}

type CompoundSelector struct {
	Tag        string
	ID         string
	Classes    []string
	Attributes []AttributeSelector
}

type SelectorStep struct {
	Combinator Combinator
	Compound   CompoundSelector
}

type Selector struct {
	Steps []SelectorStep
}

func (s *SelectorStep) Matches(node *DOMNode) bool {
	if node == nil {
		return false
	}

	if s.Combinator != CombinatorNone {
		switch s.Combinator {
		case CombinatorDescendant:
			current := node.Parent
			for current != nil {
				if s.Compound.Matches(current) {
					return true
				}
				current = current.Parent
			}
			return false
		case CombinatorChild:
			return s.Compound.Matches(node.Parent)
		case CombinatorAdjacentSibling:
			if node.Parent == nil {
				return false
			}
			siblings := node.Parent.Children
			for i, sibling := range siblings {
				if sibling == node && i > 0 {
					return s.Compound.Matches(siblings[i-1])
				}
			}
			return false
		case CombinatorGeneralSibling:
			if node.Parent == nil {
				return false
			}
			siblings := node.Parent.Children
			for _, sibling := range siblings {
				if sibling == node {
					break
				}
				if s.Compound.Matches(sibling) {
					return true
				}
			}
			return false
		}
	}

	if s.Compound.Tag != "" && s.Compound.Tag != node.Tag {
		return false
	}
	if s.Compound.ID != "" && s.Compound.ID != node.ID {
		return false
	}
	for _, class := range s.Compound.Classes {
		found := slices.Contains(node.Classes, class)
		if !found {
			return false
		}
	}

	for _, attr := range s.Compound.Attributes {
		nodeAttrValue, exists := node.Attributes[attr.Name]
		switch attr.Operator {
		case AttrOperatorExists:
			if !exists {
				return false
			}
		case AttrOperatorEquals:
			if !exists || nodeAttrValue != attr.Value {
				return false
			}
		case AttrOperatorIncludes:
			if !exists || !strings.Contains(nodeAttrValue, attr.Value) {
				return false
			}
		case AttrOperatorDashMatch:
			if !exists || (nodeAttrValue != attr.Value && !strings.HasPrefix(nodeAttrValue, attr.Value+"-")) {
				return false
			}
		case AttrOperatorPrefixMatch:
			if !exists || !strings.HasPrefix(nodeAttrValue, attr.Value) {
				return false
			}
		case AttrOperatorSuffixMatch:
			if !exists || !strings.HasSuffix(nodeAttrValue, attr.Value) {
				return false
			}
		case AttrOperatorSubstringMatch:
			if !exists || !strings.Contains(nodeAttrValue, attr.Value) {
				return false
			}
		}
	}

	return true
}

func (c *CompoundSelector) Matches(node *DOMNode) bool {
	if node == nil {
		return false
	}

	if c.Tag != "" && c.Tag != node.Tag {
		return false
	}

	if c.ID != "" && c.ID != node.ID {
		return false
	}

	for _, class := range c.Classes {
		found := slices.Contains(node.Classes, class)
		if !found {
			return false
		}
	}

	for _, attr := range c.Attributes {
		nodeAttrValue, exists := node.Attributes[attr.Name]
		switch attr.Operator {
		case AttrOperatorExists:
			if !exists {
				return false
			}
		case AttrOperatorEquals:
			if !exists || nodeAttrValue != attr.Value {
				return false
			}
		case AttrOperatorIncludes:
			if !exists || !strings.Contains(nodeAttrValue, attr.Value) {
				return false
			}
		case AttrOperatorDashMatch:
			if !exists || (nodeAttrValue != attr.Value && !strings.HasPrefix(nodeAttrValue, attr.Value+"-")) {
				return false
			}
		case AttrOperatorPrefixMatch:
			if !exists || !strings.HasPrefix(nodeAttrValue, attr.Value) {
				return false
			}
		case AttrOperatorSuffixMatch:
			if !exists || !strings.HasSuffix(nodeAttrValue, attr.Value) {
				return false
			}
		case AttrOperatorSubstringMatch:
			if !exists || !strings.Contains(nodeAttrValue, attr.Value) {
				return false
			}
		}
	}
	return true
}

func (s *Selector) String() string {
	var parts []string
	for _, step := range s.Steps {
		var part string
		if step.Combinator != CombinatorNone {
			part += string(step.Combinator)
		}
		if step.Compound.Tag != "" {
			part += step.Compound.Tag
		}
		if step.Compound.ID != "" {
			part += "#" + step.Compound.ID
		}
		for _, class := range step.Compound.Classes {
			part += "." + class
		}
		for _, attr := range step.Compound.Attributes {
			switch attr.Operator {
			case AttrOperatorExists:
				part += fmt.Sprintf("[%s]", attr.Name)
			case AttrOperatorEquals:
				part += fmt.Sprintf("[%s=\"%s\"]", attr.Name, attr.Value)
			case AttrOperatorIncludes:
				part += fmt.Sprintf("[%s~=\"%s\"]", attr.Name, attr.Value)
			case AttrOperatorDashMatch:
				part += fmt.Sprintf("[%s|=\"%s\"]", attr.Name, attr.Value)
			case AttrOperatorPrefixMatch:
				part += fmt.Sprintf("[%s^=\"%s\"]", attr.Name, attr.Value)
			case AttrOperatorSuffixMatch:
				part += fmt.Sprintf("[%s$=\"%s\"]", attr.Name, attr.Value)
			case AttrOperatorSubstringMatch:
				part += fmt.Sprintf("[%s*=\"%s\"]", attr.Name, attr.Value)
			}
		}
		parts = append(parts, part)
	}
	return strings.Join(parts, "")
}
