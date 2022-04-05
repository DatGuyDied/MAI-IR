package robotic

import (
	"encoding/xml"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"strings"
	"time"
)

type Robotic struct {
}

func NewRobotic() *Robotic {
	return &Robotic{}
}

type UrlSet struct {
	Url []Url `xml:"url"`
}

type Url struct {
	Loc string `xml:"loc"`
}

func (r *Robotic) GetLinkFromXML(url string) ([]string, error) {
	resp, err := http.Get(url)
	if err != nil {
		return nil, err
	}

	var content UrlSet
	err = xml.NewDecoder(resp.Body).Decode(&content)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	ret := make([]string, 0, len(content.Url))
	for _, l := range content.Url {
		ret = append(ret, l.Loc)
	}

	return ret, nil
}

func (r *Robotic) DownloadFromXML(url, path string) error {
	links, err := r.GetLinkFromXML(url)
	if err != nil {
		return err
	}

	for i, l := range links {
		name := strings.ReplaceAll(
			strings.TrimPrefix(l, "https://anilist.co/manga/"),
			"/",
			"-",
		)

		fmt.Printf("\r%d%%(%d/%d)", i*100/len(links), i, len(links))
		file, err := os.Create(path + name + ".html")
		if err != nil {
			return err
		}

		resp, err := http.Get(l)
		if err != nil {
			log.Println(err)
		}

		if resp.StatusCode > 299 {
			log.Printf("non-2xx status code: %v", resp)
		}

		_, err = io.Copy(file, resp.Body)
		if err != nil {
			log.Println(err)
		}

		resp.Body.Close()
		file.Close()
		time.Sleep(time.Millisecond * 1200)
	}

	fmt.Println()
	return nil
}
