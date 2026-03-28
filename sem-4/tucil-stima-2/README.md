# Tucil 2 Strategi Algoritma

Program ini adalah voxelizer berbasis octree untuk file 3D `.obj`.
Program membaca mesh segitiga dari file input, membangun octree hingga kedalaman tertentu (`max-depth`), lalu mengekspor hasil voxelisasi ke file `.obj` baru dengan nama `<nama-file>-voxed-<max-depth>.obj`.

## Requirement Program
- Go (versi 1.25.5 dari `go.mod`)

## Compile and Run
Dari root project, jalankan sesuai OS:

- Linux/macOS:

	```bash
	mkdir -p bin
	go build -o bin/voxelizer ./src
	```

- Windows:

	```powershell
	mkdir bin
	go build -o bin/voxelizer.exe ./src
	```

## Cara Menjalankan dan Menggunakan Program
Jalankan langsung tanpa kompilasi:

```bash
go run ./src <input.obj> <max-depth>
```

Atau jalankan binary hasil kompilasi:

- Linux/macOS:

	```bash
	./bin/voxelizer <input.obj> <max-depth>
	```

- Windows:

	```powershell
	./bin/voxelizer.exe <input.obj> <max-depth>
	```

Contoh:

```bash
go run ./src test/cow.obj 5
```

Parameter:
- `<input.obj>`: path file `.obj` input
- `<max-depth>`: kedalaman octree (bilangan bulat positif)

Output:
- File `.obj` hasil voxelisasi disimpan pada path yang sama dengan input, dengan suffix `-voxed-<max-depth>.obj`
- Program menampilkan statistik jumlah voxel, vertex, faces, node octree, dan waktu eksekusi (dengan metode konkuren)
- Program menampilkan viewer dari file `.obj` hasil voxelisasi

## Author
| Nama | NIM |
|------|-----|
| Moreno Syawali Ganda Sugita | 13524096 |
| Nathaniel Christian | 13524122 |
