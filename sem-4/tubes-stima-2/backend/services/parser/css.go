package parser

import (
	"backend/models"
	"fmt"
	"strings"
	"unicode"
)

type selectorParser struct {
	src string
	pos int
}

func ParseCSSSelector(input string) (models.Selector, error) {
	p := &selectorParser{src: strings.TrimSpace(input)}
	if p.src == "" {
		return models.Selector{}, fmt.Errorf("empty selector")
	}

	sel, err := p.parseSelector(true)
	if err != nil {
		return models.Selector{}, err
	}

	p.skipSpaces()
	if !p.end() {
		return models.Selector{}, fmt.Errorf("unexpected token %q at position %d", p.peek(), p.pos)
	}
	return sel, nil
}

func ParseSelectorList(input string) ([]models.Selector, error) {
	p := &selectorParser{src: input}
	var selectors []models.Selector

	for {
		p.skipSpaces()
		if p.end() {
			break
		}

		sel, err := p.parseSelector(true)
		if err != nil {
			return nil, err
		}
		selectors = append(selectors, sel)

		p.skipSpaces()
		if p.end() {
			break
		}
		if p.peek() != ',' {
			return nil, fmt.Errorf("expected ',' at position %d", p.pos)
		}
		p.pos++
	}

	if len(selectors) == 0 {
		return nil, fmt.Errorf("empty selector")
	}
	return selectors, nil
}

func (p *selectorParser) parseSelector(stopOnComma bool) (models.Selector, error) {
	steps := make([]models.SelectorStep, 0, 4)

	first, err := p.parseCompound()
	if err != nil {
		return models.Selector{}, err
	}
	steps = append(steps, models.SelectorStep{
		Combinator: models.CombinatorNone,
		Compound:   first,
	})

	for {
		spaces := p.skipSpaces()
		if p.end() || (stopOnComma && p.peek() == ',') {
			break
		}

		comb, explicit := p.readCombinator()
		if explicit {
			p.skipSpaces()
			if p.end() || (stopOnComma && p.peek() == ',') {
				return models.Selector{}, fmt.Errorf("missing selector after combinator at position %d", p.pos)
			}
		} else if spaces > 0 {
			comb = models.CombinatorDescendant
		} else {
			return models.Selector{}, fmt.Errorf("expected combinator at position %d", p.pos)
		}

		compound, err := p.parseCompound()
		if err != nil {
			return models.Selector{}, err
		}

		steps = append(steps, models.SelectorStep{
			Combinator: comb,
			Compound:   compound,
		})
	}

	return models.Selector{Steps: steps}, nil
}

func (p *selectorParser) parseCompound() (models.CompoundSelector, error) {
	var c models.CompoundSelector
	found := false

	if p.end() {
		return c, fmt.Errorf("unexpected end of selector at position %d", p.pos)
	}

	if p.peek() == '*' {
		p.pos++
		found = true
	} else if isIdentifierStart(p.peek()) {
		c.Tag = strings.ToLower(p.readIdent())
		found = true
	}

	for !p.end() {
		switch p.peek() {
		case '#':
			p.pos++
			id := p.readIdent()
			if id == "" {
				return c, fmt.Errorf("expected id after '#' at position %d", p.pos)
			}
			if c.ID != "" {
				return c, fmt.Errorf("multiple id selectors in one compound at position %d", p.pos)
			}
			c.ID = id
			found = true
		case '.':
			p.pos++
			className := p.readIdent()
			if className == "" {
				return c, fmt.Errorf("expected class name after '.' at position %d", p.pos)
			}
			c.Classes = append(c.Classes, className)
			found = true
		case '[':
			attr, err := p.readAttributeSelector()
			if err != nil {
				return c, err
			}
			c.Attributes = append(c.Attributes, attr)
			found = true
		case ':':
			return c, fmt.Errorf("pseudo selectors are not supported yet (position %d)", p.pos)
		default:
			if isCombinator(p.peek()) {
				if !found {
					return c, fmt.Errorf("expected selector token at position %d", p.pos)
				}
				return c, nil
			}
			return c, fmt.Errorf("unexpected character %q at position %d", p.peek(), p.pos)
		}
	}

	if !found {
		return c, fmt.Errorf("expected selector token at position %d", p.pos)
	}
	return c, nil
}

func (p *selectorParser) readAttributeSelector() (models.AttributeSelector, error) {
	var res models.AttributeSelector

	p.pos++
	p.skipSpaces()

	name := p.readIdent()
	if name == "" {
		return res, fmt.Errorf("expected attribute name at position %d", p.pos)
	}
	res.Name = strings.ToLower(name)

	p.skipSpaces()
	if p.end() {
		return res, fmt.Errorf("unterminated attribute selector at position %d", p.pos)
	}

	if p.peek() == ']' {
		p.pos++
		res.Operator = models.AttrOperatorExists
		return res, nil
	}

	operator, err := p.readAttributeOperator()
	if err != nil {
		return res, err
	}
	res.Operator = operator

	p.skipSpaces()
	value, err := p.readAttributeValue()
	if err != nil {
		return res, err
	}
	res.Value = value

	p.skipSpaces()
	if p.end() || p.peek() != ']' {
		return res, fmt.Errorf("expected ']' at position %d", p.pos)
	}
	p.pos++
	return res, nil
}

func (p *selectorParser) readAttributeOperator() (models.AttrOperator, error) {
	if p.match("~=") {
		return models.AttrOperatorIncludes, nil
	}
	if p.match("|=") {
		return models.AttrOperatorDashMatch, nil
	}
	if p.match("^=") {
		return models.AttrOperatorPrefixMatch, nil
	}
	if p.match("$=") {
		return models.AttrOperatorSuffixMatch, nil
	}
	if p.match("*=") {
		return models.AttrOperatorSubstringMatch, nil
	}
	if p.match("=") {
		return models.AttrOperatorEquals, nil
	}
	return models.AttrOperatorExists, fmt.Errorf("expected attribute operator at position %d", p.pos)
}

func (p *selectorParser) readAttributeValue() (string, error) {
	if p.end() {
		return "", fmt.Errorf("expected attribute value at position %d", p.pos)
	}

	if p.peek() == '"' || p.peek() == '\'' {
		return p.readQuotedString()
	}

	start := p.pos
	for !p.end() {
		ch := p.peek()
		if unicode.IsSpace(rune(ch)) || ch == ']' {
			break
		}
		p.pos++
	}

	if p.pos == start {
		return "", fmt.Errorf("expected attribute value at position %d", p.pos)
	}
	return p.src[start:p.pos], nil
}

func (p *selectorParser) readQuotedString() (string, error) {
	quote := p.peek()
	p.pos++

	var b strings.Builder
	for !p.end() {
		ch := p.peek()
		p.pos++

		if ch == quote {
			return b.String(), nil
		}
		if ch == '\\' && !p.end() {
			b.WriteByte(p.peek())
			p.pos++
			continue
		}

		b.WriteByte(ch)
	}

	return "", fmt.Errorf("non-terminated quoted string at position %d", p.pos)
}

func (p *selectorParser) readCombinator() (models.Combinator, bool) {
	if p.end() {
		return models.CombinatorNone, false
	}

	switch p.peek() {
	case '>':
		p.pos++
		return models.CombinatorChild, true
	case '+':
		p.pos++
		return models.CombinatorAdjacentSibling, true
	case '~':
		p.pos++
		return models.CombinatorGeneralSibling, true
	default:
		return models.CombinatorNone, false
	}
}

func (p *selectorParser) match(s string) bool {
	if len(p.src)-p.pos < len(s) {
		return false
	}
	if p.src[p.pos:p.pos+len(s)] != s {
		return false
	}
	p.pos += len(s)
	return true
}

func (p *selectorParser) readIdent() string {
	if p.end() || !isIdentifierStart(p.peek()) {
		return ""
	}

	start := p.pos
	p.pos++
	for !p.end() && isIdentifierPart(p.peek()) {
		p.pos++
	}

	return p.src[start:p.pos]
}

func (p *selectorParser) skipSpaces() int {
	start := p.pos
	for !p.end() && unicode.IsSpace(rune(p.peek())) {
		p.pos++
	}
	return p.pos - start
}

func (p *selectorParser) peek() byte {
	if p.end() {
		return 0
	}
	return p.src[p.pos]
}

func (p *selectorParser) end() bool {
	return p.pos >= len(p.src)
}

func isCombinator(ch byte) bool {
	return ch == 0 || ch == ',' || ch == '>' || ch == '+' || ch == '~' || unicode.IsSpace(rune(ch))
}

func isIdentifierStart(ch byte) bool {
	return ch == '_' || ch == '-' || isAlpha(ch)
}

func isIdentifierPart(ch byte) bool {
	return isIdentifierStart(ch) || isNumeric(ch)
}

func isAlpha(ch byte) bool {
	return (ch >= 'a' && ch <= 'z') ||
		(ch >= 'A' && ch <= 'Z')
}

func isNumeric(ch byte) bool {
	return ch >= '0' && ch <= '9'
}
