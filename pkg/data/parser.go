package data

import (
	"os"
	"regexp"
	"strings"
	"unicode"
)

type Parser struct {
	dir string
}

func NewParser(dir string) *Parser {
	return &Parser{
		dir: dir,
	}
}

func (p *Parser) GetFiles() ([]string, error) {
	dirEntry, err := os.ReadDir(p.dir)
	if err != nil {
		return nil, err
	}

	ret := make([]string, 0)
	for _, e := range dirEntry {
		if !e.IsDir() {
			ret = append(ret, p.dir+"/"+e.Name())
		} else {
			fromDir, err := NewParser(p.dir + "/" + e.Name()).GetFiles()
			if err != nil {
				return nil, err
			}

			ret = append(ret, fromDir...)
		}
	}

	return ret, nil
}

type Content struct {
	ID          string
	Title       string
	Description string
}

func (c Content) String() string {
	return c.ID + "\t" + c.Title + "\t" + c.Description
}

func (p *Parser) ToLowerASCII(s string) string {
	str := strings.Builder{}
	s = strings.ToLower(s)
	reg := regexp.MustCompile("x[0-9]+")
	s = reg.ReplaceAllString(s, "")

	for _, r := range s {
		if r < unicode.MaxASCII && (unicode.IsLetter(r) || unicode.IsDigit(r)) {
			str.WriteRune(r)
		}

		if unicode.IsSpace(r) {
			str.WriteRune(' ')
		}

	}

	return str.String()
}

func (p *Parser) GetID(fileName string) string {
	f, _ := os.Lstat(fileName)
	reg := regexp.MustCompile("[0-9]*")

	return reg.FindString(f.Name())
}

func (p *Parser) GetTitle(data []byte) string {
	prefix := "<meta data-vue-meta=\"true\" property=\"og:title\" content=\""
	suffix := "\"><meta data-vue-meta=\"true\" property=\"og:description\" content=\""

	reg := regexp.MustCompile(prefix + "(.|\n)*" + suffix)
	s := reg.FindString(string(data))

	title := strings.TrimPrefix(
		strings.TrimSuffix(
			s,
			suffix,
		),
		prefix,
	)

	return p.ToLowerASCII(title)
}

func (p *Parser) GetDescription(data []byte) string {
	prefix := "\"><meta data-vue-meta=\"true\" property=\"og:description\" content=\""
	suffix := "\"><meta data-vue-meta=\"true\" property=\"og:image\" content=\""

	reg := regexp.MustCompile(prefix + "(.|\n)*" + suffix)
	s := reg.FindString(string(data))

	description := strings.TrimPrefix(
		strings.TrimSuffix(
			s,
			suffix,
		),
		prefix,
	)

	return p.ToLowerASCII(description)
}

func (p *Parser) GetContent(file string) (Content, error) {
	f, err := os.ReadFile(file)
	if err != nil {
		return Content{}, err
	}

	return Content{
		ID:          p.GetID(file),
		Title:       p.GetTitle(f),
		Description: p.GetDescription(f),
	}, nil
}

func (p *Parser) GetContents() ([]Content, error) {
	files, err := p.GetFiles()
	if err != nil {
		return nil, err
	}

	contents := make([]Content, 0, len(files))

	for _, f := range files {
		content, err := p.GetContent(f)
		if err != nil {
			return nil, err
		}

		contents = append(contents, content)
	}

	return contents, nil
}
