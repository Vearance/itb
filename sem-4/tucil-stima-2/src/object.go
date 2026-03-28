package main

func getBoundingBox(vertex []Vertex) BoundingBox {
	var min Vertex = vertex[0]
	var max Vertex = vertex[0]

	for _, coor := range vertex {
		if coor.X < min.X {
			min.X = coor.X
		}
		if coor.Y < min.Y {
			min.Y = coor.Y
		}
		if coor.Z < min.Z {
			min.Z = coor.Z
		}

		if coor.X > max.X {
			max.X = coor.X
		}
		if coor.Y > max.Y {
			max.Y = coor.Y
		}
		if coor.Z > max.Z {
			max.Z = coor.Z
		}
	}

	return BoundingBox{min, max}
}

func getBoundingCube(bounding BoundingBox) Cube {
	x := bounding.Max.X - bounding.Min.X
	y := bounding.Max.Y - bounding.Min.Y
	z := bounding.Max.Z - bounding.Min.Z

	max := x
	if y > max {
		max = y
	}
	if z > max {
		max = z
	}

	return Cube{bounding.Min, max}
}

func min3(a, b, c float64) float64 {
    if a < b && a < c { 
		return a 
	}
    if b < c { 
		return b 
	}
    return c
}

func max3(a, b, c float64) float64 {
    if a > b && a > c {
		return a
	}
    if b > c {
		return b 
	}
    return c
}

func getBoundingTriangle(triangle Triangle) BoundingBox {
    min := Vertex{
        X: min3(triangle.V1.X, triangle.V2.X, triangle.V3.X),
        Y: min3(triangle.V1.Y, triangle.V2.Y, triangle.V3.Y),
        Z: min3(triangle.V1.Z, triangle.V2.Z, triangle.V3.Z),
    }

    max := Vertex{
        X: max3(triangle.V1.X, triangle.V2.X, triangle.V3.X),
        Y: max3(triangle.V1.Y, triangle.V2.Y, triangle.V3.Y),
        Z: max3(triangle.V1.Z, triangle.V2.Z, triangle.V3.Z),
    }

    return BoundingBox{min, max}
}

// check intersection of two bounding boxes
func isIntersect(a, b BoundingBox) bool {
    return (
		(a.Min.X <= b.Max.X && a.Max.X >= b.Min.X) &&
        (a.Min.Y <= b.Max.Y && a.Max.Y >= b.Min.Y) &&
        (a.Min.Z <= b.Max.Z && a.Max.Z >= b.Min.Z))
}

func cubeToBox(cube Cube) BoundingBox {
    return BoundingBox{cube.Min, Vertex{cube.Min.X+cube.Size, cube.Min.Y+cube.Size, cube.Min.Z+cube.Size}}
}