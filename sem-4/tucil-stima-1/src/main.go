package main

import (
	"fmt"
	"time"
)

func main() {
    fmt.Println("Queens Solver Brute Force Approach")

	// input file name
	var filename string
	fmt.Print("Input nama file (no whitespace, put inside test/ folder): ")
	fmt.Scan(&filename)

	// construct board and read file
	filepath := "../test/" + filename
	board, err := constructBoard(filepath) // constructBoard function (param filename)
	if err != nil {
		fmt.Println("Error: ", err)
		return
	}

	// initialized counter var
	var counter int

	// validation
	if !board.isValid() {
		fmt.Println("Board tidak valid.")
		return
	}


	// do solve
	// bruteforceSolve function (param board from input and counter)
	var useAlgo string
	fmt.Print("Pakai algoritma exhaustive (1) atau backtracking (2)? Input 1 atau 2: ")
	fmt.Scan(&useAlgo)

	var found bool

	// start var to count duration
	startTime := time.Now()

	if useAlgo == "1" {
		found = Solve(board, &counter)
	} else {
		found = SolveEffective(board, &counter)
	}

	duration := time.Since(startTime)

	clearScreen()
	if found {
		printBoard(board)
	} else {
		fmt.Println("Tidak ada solusi yang ditemukan.")
	}
	
	fmt.Println()
	fmt.Printf("Waktu pencarian: %.3f ms\n", float64(duration.Microseconds())/1000.0)
	fmt.Println("Banyak kasus yang ditinjau:", counter)


	// save function optional
	var save string
	fmt.Print("Apakah Anda ingin menyimpan solusi? (Y/N): ")
	fmt.Scan(&save)

	filepath = "../test/solution/" + filename
	if save == "Y" {
		err := saveBoard(board, filepath)
		if err != nil {
			fmt.Println("Error:", err)
		} else {
			fmt.Println("File berhasil disimpan di:", filepath)
		}
	}
}