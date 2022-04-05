package main

import (
	"MAI-IR/pkg/data"
	"fmt"
)

func main() {
	v, err := data.NewParser("data/raw").GetContents()
	if err != nil {
		panic(err)
	}

	for _, val := range v {
		fmt.Printf("%s\n", val.String())
	}
}
