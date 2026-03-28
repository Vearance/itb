package main

// one cube would be divided into 8 sub-cube
func divideCube(cube Cube) [8]Cube {
	var child [8]Cube

	mid := cube.Size / 2
    x := cube.Min.X
    y := cube.Min.Y
    z := cube.Min.Z

    points := [8][3]float64{
        {0, 0, 0},
        {mid, 0, 0},
        {0, mid, 0},
        {mid, mid, 0},
        {0, 0, mid},
        {mid, 0, mid},
        {0, mid, mid},
        {mid, mid, mid},
    }

	for i := 0; i < 8; i++ {
        child[i] = Cube{Vertex{x+points[i][0], y+points[i][1], z+points[i][2],}, mid}
    }

	return child
}

func intersectCubeTriangle(c Cube, triangles []Triangle) bool {
    cubeBox := cubeToBox(c)

    for _, t := range triangles {
        triBox := getBoundingTriangle(t)

        if isIntersect(cubeBox, triBox) {
            return true
        }
    }

    return false
}

func buildTree(cube Cube, triangles []Triangle, currDepth int, maxDepth int, done map[int]int, remove map[int]int) *OctreeNode {
    if !intersectCubeTriangle(cube, triangles) {
        if currDepth > 0 && remove != nil {
            remove[currDepth]++
        }
        return nil
    }

    node := &OctreeNode{Cube: cube, triangles: triangles}
    if currDepth > 0 && done != nil {
        done[currDepth]++
    }

    // base case
    if currDepth == maxDepth {
        node.isLastNode = true
        return node
    }

    // divide    
    dividedCubes := divideCube(cube)

    for i := 0; i < 8; i++ {
        node.Child[i] = buildTree(dividedCubes[i], triangles, currDepth+1, maxDepth, done, remove)
    }

    return node
}

func getVoxel(node *OctreeNode, voxels *[]Voxel) {
    if node == nil {
        return
    }

    if node.isLastNode {
        *voxels = append(*voxels, Voxel{node.Cube})
        return
    }

    for i := 0; i < 8; i++ {
        getVoxel(node.Child[i], voxels)
    }
}