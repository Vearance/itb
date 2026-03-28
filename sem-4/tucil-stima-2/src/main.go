package main

import (
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"sync"
	"time"
)

func outputPath(input string, depth int) string {
	ext := filepath.Ext(input)
	base := input
	if ext != "" {
		base = input[:len(input)-len(ext)]
	}
	return fmt.Sprintf("%s-voxed-%d.obj", base, depth)
}

func main() {
	if len(os.Args) < 3 {
        fmt.Println("Usage: go run ./src <input.obj> <max-depth>")
        os.Exit(1)
    }

    input := os.Args[1]
    depth, err := strconv.Atoi(os.Args[2])
    if err != nil || depth < 0 {
        fmt.Println("Error: <max-depth> can't be negative")
        os.Exit(1)
    }

    output := outputPath(input, depth)
    absOutput, err := filepath.Abs(output)
    if err == nil {
        output = absOutput
    }

    start := time.Now()

    vertex, triangles, err := loadObject(input)
    if err != nil {
        fmt.Println("Error loading OBJ:", err)
        os.Exit(1)
    }

	bounding := getBoundingBox(vertex)
	cube := getBoundingCube(bounding)

	done := make(map[int]int)
	removed := make(map[int]int)
	var mu sync.Mutex
	root := buildTreeConcurrent(cube, triangles, 0, depth, done, removed, &mu)

	var voxels []Voxel
	getVoxel(root, &voxels)

	meshVertex, meshFaces := voxelToMesh(voxels)

    if err := saveObject(output, meshVertex, meshFaces); err != nil {
        fmt.Println("Error writing OBJ:", err)
        os.Exit(1)
    }

    fmt.Println("Banyaknya voxel yang terbentuk:", len(voxels))
    fmt.Println("Banyaknya vertex yang terbentuk:", len(meshVertex))
    fmt.Println("Banyaknya faces yang terbentuk:", len(meshFaces))

	fmt.Println("Statistik node octree yang terbentuk:")
	for i := 1; i <= depth; i++ {
		fmt.Printf("%d : %d\n", i, done[i])
	}

	fmt.Println("Statistik node yang tidak perlu ditelusuri:")
	for i := 1; i <= depth; i++ {
		fmt.Printf("%d : %d\n", i, removed[i])
	}

	fmt.Println("Kedalaman octree:", depth)
	fmt.Println("Lama waktu program berjalan:", time.Since(start))
    fmt.Println("Membuka 3D Object Viewer...")
	showViewer(meshVertex, meshFaces)
    fmt.Println("Path output:", output)
}