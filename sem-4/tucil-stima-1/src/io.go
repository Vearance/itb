package main

import (
	"bufio"
	"fmt"
	"os"
	"time"
)

func constructBoard(filename string) (*Board, error) {
	file, err := os.Open(filename)
	if err != nil {
		return nil, err
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)

	//initialize colorMatrix
	var colorMatrix [][]rune

	for scanner.Scan() {
		line := scanner.Text() // current line as string
		row := []rune(line) // take this row line to a char array and put it into matrix
		colorMatrix = append(colorMatrix, row)
	}

	if err := scanner.Err(); err != nil {
		return nil, err
	}

	return newBoard(colorMatrix), nil
}

func saveBoard(b *Board, filename string) error {
	file, err := os.Create(filename) // if it exists, it will be replaced
	if err != nil {
		return err
	}
	defer file.Close()

	for i := 0; i < b.size; i++ {
		for j:= 0; j < b.size; j++ {
			if b.queen[i][j] {
				_, err = file.WriteString("#")
			} else {
				_, err = file.WriteString(string(b.color[i][j]))
			}
		}

		_, err = file.WriteString("\n")
		if err != nil {
			return err
		}
	}

	return nil
}

func printBoard(b *Board) {
	for i := 0; i < b.size; i++ {
		for j := 0; j < b.size; j++ {
			if b.queen[i][j] {
				fmt.Printf("#")
			} else {
				fmt.Print(string(b.color[i][j]))
			}
		}
		fmt.Println()
	}
}

func liveBoard(b *Board, counter int) {
	if counter%100 != 0 {
		return
	}

	clearScreen()

	for i := 0; i < b.size; i++ {
		for j := 0; j < b.size; j++ {
			if b.queen[i][j] {
				fmt.Printf("#")
			} else {
				fmt.Print(string(b.color[i][j]))
			}
		}
		fmt.Println()
	}

	time.Sleep(100 * time.Millisecond)
}

func clearScreen() {
    fmt.Print("\033[H\033[2J")
}