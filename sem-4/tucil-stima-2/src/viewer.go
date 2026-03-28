package main

import (
	"math"
	"sort"
	rl "github.com/gen2brain/raylib-go/raylib"
)

type FaceToDraw struct {
	P1, P2, P3 rl.Vector2
	Depth      float64
}

func showViewer(vertices []Vertex, faces [][3]int) {
	rl.SetTraceLogLevel(rl.LogNone)
	sw, sh := 800, 600
	rl.InitWindow(int32(sw), int32(sh), "3D Object Viewer")
	defer rl.CloseWindow()
	rl.SetTargetFPS(60)

	// Calculate center point & object size
	var minX, minY, minZ = vertices[0].X, vertices[0].Y, vertices[0].Z
	var maxX, maxY, maxZ = minX, minY, minZ
	for _, v := range vertices {
		minX = math.Min(minX, v.X); maxX = math.Max(maxX, v.X)
		minY = math.Min(minY, v.Y); maxY = math.Max(maxY, v.Y)
		minZ = math.Min(minZ, v.Z); maxZ = math.Max(maxZ, v.Z)
	}
	cX, cY, cZ := (minX+maxX)/2, (minY+maxY)/2, (minZ+maxZ)/2
	
	// Calculate bounding box
	spanX, spanY, spanZ := maxX-minX, maxY-minY, maxZ-minZ
	maxSpan := math.Max(spanX, math.Max(spanY, spanZ))
	
	radius := maxSpan * 1.5 
	if radius == 0 { radius = 10.0 }

	var yaw, pitch float64 = 0.5, 0.3

	for !rl.WindowShouldClose() {
		wheel := float64(rl.GetMouseWheelMove())
		if wheel != 0 {
			radius -= wheel * (maxSpan * 0.1)
		}

		if radius < maxSpan*0.2 { radius = maxSpan * 0.2 }

		if rl.IsMouseButtonDown(rl.MouseLeftButton) {
			delta := rl.GetMouseDelta()
			if math.Abs(float64(delta.X)) > 0.5 || math.Abs(float64(delta.Y)) > 0.5 {
				yaw += float64(delta.X) * 0.01
				pitch -= float64(delta.Y) * 0.01
			}
			if pitch > 1.5 { pitch = 1.5 }
			if pitch < -1.5 { pitch = -1.5 }
		}

		var toDraw []FaceToDraw
		fov := 1.5

		for _, f := range faces {
			var screenPts [3]rl.Vector2
			var avgZ float64
			visible := true

			for i := 0; i < 3; i++ {
				v := vertices[f[i]-1]
				x, y, z := v.X-cX, v.Y-cY, v.Z-cZ

				// Yaw rotation
				x1 := x*math.Cos(yaw) - z*math.Sin(yaw)
				z1 := x*math.Sin(yaw) + z*math.Cos(yaw)
				y1 := y

				// Pitch rotation
				y2 := y1*math.Cos(pitch) - z1*math.Sin(pitch)
				z2 := y1*math.Sin(pitch) + z1*math.Cos(pitch)
				x2 := x1

				zFinal := z2 + radius

				if zFinal <= 0.1 {
					visible = false
					break
				}

				projX := (x2 * fov) / zFinal
				projY := (y2 * fov) / zFinal

				screenPts[i] = rl.Vector2{
					X: float32(float64(sw)/2 + projX*float64(sh)/2),
					Y: float32(float64(sh)/2 - projY*float64(sh)/2),
				}
				avgZ += zFinal
			}

			if visible {
				toDraw = append(toDraw, FaceToDraw{screenPts[0], screenPts[1], screenPts[2], avgZ / 3.0})
			}
		}

		sort.Slice(toDraw, func(i, j int) bool {
			return toDraw[i].Depth > toDraw[j].Depth
		})

		rl.BeginDrawing()
		rl.ClearBackground(rl.RayWhite)

		for _, tri := range toDraw {
			rl.DrawTriangle(tri.P1, tri.P2, tri.P3, rl.SkyBlue)
			rl.DrawTriangle(tri.P1, tri.P3, tri.P2, rl.SkyBlue)
			rl.DrawLineV(tri.P1, tri.P2, rl.DarkBlue)
			rl.DrawLineV(tri.P2, tri.P3, rl.DarkBlue)
			rl.DrawLineV(tri.P3, tri.P1, rl.DarkBlue)
		}
		
		rl.EndDrawing()
	}
}