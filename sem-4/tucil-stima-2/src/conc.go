package main

import "sync"

// For ur memory
const MaxConcurrentDepth = 4

func buildTreeConcurrent(cube Cube, triangles []Triangle, currDepth int, maxDepth int, done map[int]int, remove map[int]int, mu *sync.Mutex) *OctreeNode {
	if !intersectCubeTriangle(cube, triangles) {
		if currDepth > 0 && remove != nil {
			mu.Lock()
			remove[currDepth]++
			mu.Unlock()
		}
		return nil
	}

	node := &OctreeNode{Cube: cube, triangles: triangles}
	if currDepth > 0 && done != nil {
		mu.Lock()
		done[currDepth]++
		mu.Unlock()
	}

	if currDepth == maxDepth {
		node.isLastNode = true
		return node
	}

	dividedCubes := divideCube(cube)

	if currDepth < MaxConcurrentDepth {
		var wg sync.WaitGroup
		for i := 0; i < 8; i++ {
			wg.Add(1)
			go func(index int) {
				defer wg.Done()
				node.Child[index] = buildTreeConcurrent(dividedCubes[index], triangles, currDepth+1, maxDepth, done, remove, mu)
			}(i)
		}
		wg.Wait()
	} else {
		for i := 0; i < 8; i++ {
			node.Child[i] = buildTreeConcurrent(dividedCubes[i], triangles, currDepth+1, maxDepth, done, remove, mu)
		}
	}

	return node
}