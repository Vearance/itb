package main

// in .obj files Vertex is written as 'v' or 'vertex'
type Vertex struct {
    X, Y, Z float64
}

// in .obj files Triangle is written as 'f' or 'face'
type Triangle struct {
    V1, V2, V3 Vertex
}

// Axis-Aligned Bounding Box (AABB)
type BoundingBox struct {
	Min, Max Vertex
}

// bounding box is not a cube, so we'll make it a cube
type Cube struct {
	Min Vertex  // smallest coordinate of the cube
	Size float64
}

type OctreeNode struct {
	Cube Cube
	Child [8]*OctreeNode
	isLastNode bool
	triangles []Triangle
}

type Voxel struct {
	Cube Cube
}

// used as a map key to detect duplication.
type vertexKey struct {
	X, Y, Z uint64
}