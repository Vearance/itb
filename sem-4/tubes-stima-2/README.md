# Tugas Besar 2 Strategi Algoritma

Project ini terdiri dari:
- **Backend (Go + Fiber)** untuk mengambil HTML dari URL, mem-parse DOM, mem-parse selector CSS, lalu melakukan pencarian elemen.
- **Frontend (Next.js w/ Bun)** untuk visualisasi hasil traversal/pencarian.

## Penjelasan BFS dan DFS

### BFS (Breadth-First Search)
Implementasi BFS melakukan penelusuran level demi level menggunakan queue.
- Node diproses dari antrian secara FIFO.
- Pada setiap node, sistem mengecek kecocokan step selector saat ini.
- Jika step belum terakhir, node-node relasi berikutnya (berdasarkan combinator selector) dimasukkan ke queue.
- Traversal berhenti ketika jumlah hasil (`amount`) terpenuhi atau queue habis.

### DFS (Depth-First Search)
Implementasi DFS menggunakan rekursi untuk menelusuri sedalam mungkin pada cabang DOM.
- Traversal awal menelusuri seluruh node secara depth-first.
- Untuk node yang cocok, pencocokan step selector dilanjutkan rekursif ke kandidat relasi step berikutnya.
- Jika step terakhir terpenuhi, node disimpan sebagai hasil.
- Traversal berhenti ketika jumlah hasil (`amount`) terpenuhi.

## Requirements

### Backend
- Go (sesuai `go.mod`, **Go 1.26.2**)

## Installation
Jalankan dari root:

```bash
# Backend dependencies
cd backend
go mod tidy

# Frontend dependencies
cd frontend
bun install
```

## How to run without Docker

### 1) Jalankan Backend
```bash
cd backend
go run .
```
Backend berjalan di: `http://localhost:6767`

### 2) Jalankan Frontend
```bash
cd frontend
bun run dev
```
Frontend berjalan di: `http://localhost:3000`

## How to run with Docker

```bash
# Build and run
docker compose up --build  # Now we can access our website at http://localhost:3000 and http://localhost:6767

# Stop
docker compose down
```

## Author
| Nama | NIM |
|------|-----|
| Nathaniel Christian | 13524122 |
| Azri Arzaq Pohan | 13524139 |
| Rasyad Satyatama | 13524142 |
