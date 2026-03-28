package main

import "math"

var cubeFaces = [][3]int{
	{0, 1, 2}, {0, 2, 3},
	{4, 6, 5}, {4, 7, 6},
	{0, 5, 1}, {0, 4, 5},
	{3, 2, 6}, {3, 6, 7},
	{0, 3, 7}, {0, 7, 4},
	{1, 5, 6}, {1, 6, 2},
}

func makeVertexKey(v Vertex) vertexKey {
	return vertexKey{
		X: math.Float64bits(v.X),
		Y: math.Float64bits(v.Y),
		Z: math.Float64bits(v.Z),
	}
}

func sortedFaceKey(a, b, c int) [3]int {
	if a > b {
		a, b = b, a
	}
	if b > c {
		b, c = c, b
	}
	if a > b {
		a, b = b, a
	}
	return [3]int{a, b, c}
}

func getCubeVertex(cube Cube) []Vertex {
	min := cube.Min
	max := Vertex{min.X + cube.Size, min.Y + cube.Size, min.Z + cube.Size}

	return []Vertex{
		{min.X, min.Y, min.Z},
		{max.X, min.Y, min.Z},
		{max.X, max.Y, min.Z},
		{min.X, max.Y, min.Z},
		{min.X, min.Y, max.Z},
		{max.X, min.Y, max.Z},
		{max.X, max.Y, max.Z},
		{min.X, max.Y, max.Z},
	}
}

func voxelToMesh(voxels []Voxel) ([]Vertex, [][3]int) {
	vertex := make([]Vertex, 0, len(voxels)*8)
	faces := make([][3]int, 0, len(voxels)*12)
	vertexIndex := make(map[vertexKey]int, len(voxels)*8)
	faceSet := make(map[[3]int]struct{}, len(voxels)*12)

	for _, voxel := range voxels {
		cubeVertex := getCubeVertex(voxel.Cube)

		for _, face := range cubeFaces {
			var tri [3]int

			for i := 0; i < 3; i++ {
				v := cubeVertex[face[i]]
				key := makeVertexKey(v)

				idx, found := vertexIndex[key]
				if !found {
					vertex = append(vertex, v)
					idx = len(vertex)
					vertexIndex[key] = idx
				}

				tri[i] = idx
			}

			if tri[0] == tri[1] || tri[1] == tri[2] || tri[0] == tri[2] {
				continue
			}

			sortedKey := sortedFaceKey(tri[0], tri[1], tri[2])
			if _, exists := faceSet[sortedKey]; exists {
				continue
			}

			faceSet[sortedKey] = struct{}{}
			faces = append(faces, tri)
		}
	}

	return vertex, faces
}
