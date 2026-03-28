package main

import (
	"bufio"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

func loadObject(path string) ([]Vertex, []Triangle, error) {
	if strings.ToLower(filepath.Ext(path)) != ".obj" {
		return nil, nil, fmt.Errorf("input harus file .obj")
	}

	file, err := os.Open(path)
    if err != nil {
        return nil, nil, err
    }
	defer file.Close()

	var vertex []Vertex
	var triangles []Triangle
	
	scanner := bufio.NewScanner(file)
	lineNo := 0

	for scanner.Scan() {
		lineNo++
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			continue
		}

		if strings.HasPrefix(line, "v ") {
			coordinate := strings.Fields(line)
			if len(coordinate) != 4 {
				return nil, nil, fmt.Errorf("line %d: format vertex tidak valid, harus 'v x y z'", lineNo)
			}

			x, errX := strconv.ParseFloat(coordinate[1], 64)
			y, errY := strconv.ParseFloat(coordinate[2], 64)
			z, errZ := strconv.ParseFloat(coordinate[3], 64)
			if errX != nil || errY != nil || errZ != nil {
				return nil, nil, fmt.Errorf("line %d: nilai vertex harus angka", lineNo)
			}

			vertex = append(vertex, Vertex{x, y, z})
		} else if strings.HasPrefix(line, "f ") {
			index := strings.Fields(line)
			if len(index) != 4 {
				return nil, nil, fmt.Errorf("line %d: format face tidak valid, harus 'f i j k'", lineNo)
			}
			if strings.Contains(index[1], "/") || strings.Contains(index[2], "/") || strings.Contains(index[3], "/") {
				return nil, nil, fmt.Errorf("line %d: format face harus 'f i j k' tanpa '/'", lineNo)
			}

			i, errI := strconv.Atoi(index[1])
			j, errJ := strconv.Atoi(index[2])
			k, errK := strconv.Atoi(index[3])
			if errI != nil || errJ != nil || errK != nil {
				return nil, nil, fmt.Errorf("line %d: indeks face harus integer", lineNo)
			}
			if i <= 0 || j <= 0 || k <= 0 {
				return nil, nil, fmt.Errorf("line %d: indeks face harus positif", lineNo)
			}
			if i > len(vertex) || j > len(vertex) || k > len(vertex) {
				return nil, nil, fmt.Errorf("line %d: indeks face di luar range vertex", lineNo)
			}

			// take vertex from vertex[], arr index starts from 0
			v1 := vertex[i-1]
			v2 := vertex[j-1]
			v3 := vertex[k-1]

			triangles = append(triangles, Triangle{v1, v2, v3})
		} else {
			continue;

		}
	}

	if err := scanner.Err(); err != nil {
        return nil, nil, err
    }

	if len(vertex) == 0 {
		return nil, nil, fmt.Errorf("OBJ tidak punya vertex")
	}
	if len(triangles) == 0 {
		return nil, nil, fmt.Errorf("OBJ tidak punya face")
	}

	return vertex, triangles, nil
}

func saveObject(path string, vertex []Vertex, face [][3]int) error {
	file, err := os.Create(path)
	if err != nil {
		return err
	}
	defer file.Close()

	for _, v := range vertex {
		line := "v " +
			strconv.FormatFloat(v.X, 'f', -1, 64) + " " +
			strconv.FormatFloat(v.Y, 'f', -1, 64) + " " +
			strconv.FormatFloat(v.Z, 'f', -1, 64) + "\n"
		if _, err := file.WriteString(line); err != nil {
			return err
		}
	}

	for _, face := range face {
		line := "f " +
			strconv.Itoa(face[0]) + " " +
			strconv.Itoa(face[1]) + " " +
			strconv.Itoa(face[2]) + "\n"
		if _, err := file.WriteString(line); err != nil {
			return err
		}
	}

	return nil
}