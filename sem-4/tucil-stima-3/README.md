# Tucil 3 Strategi Algoritma

Program ini adalah penyelesai (solver) untuk permainan Ice Sliding Puzzle dengan antarmuka grafis (GUI) berbasis Wails.
Program membaca peta dari file teks, kemudian mencari jalur penyelesaian menggunakan salah satu dari lima algoritma pencarian (UCS, BFS, GBFS, A*, IDA*) dengan tiga pilihan heuristik (Manhattan, Euclidean, Chebyshev), lalu menampilkan visualisasi langkah demi langkah dari solusi yang ditemukan.

## Requirements
- Go (1.25.5+)
- [Wails CLI v2](https://wails.io/docs/gettingstarted/installation/) & platform dependencies

## Setup

**Linux**
```bash
sudo apt update
sudo apt install -y build-essential pkg-config libgtk-3-dev libwebkit2gtk-4.1-dev

go install github.com/wailsapp/wails/v2/cmd/wails@latest
echo 'export PATH=$PATH:$(go env GOPATH)/bin' >> ~/.bashrc
source ~/.bashrc
```

**macOS:** Xcode Command Line Tools only.

**Windows:** Install [WebView2 Runtime](https://developer.microsoft.com/en-us/microsoft-edge/webview2/).

## Build & Run

```bash
# Build
wails build

# Run
./build/bin/solver           # Linux
./build/bin/solver.exe       # Windows
open ./build/bin/Solver.app  # macOS

# Or dev mode (hot-reload)
wails dev
```

## Usage

1. Click **Browse...** → select a map file (e.g., `test/input.txt`) → **Load** to confirm map.
2. Choose algorithm (`UCS`/`BFS`/`GBFS`/`A*`/`IDA*`) & heuristic (`H1` Manhattan/`H2` Euclidean/`H3` Chebyshev)
3. Click **Solve** → see path, cost, iterations, elapsed time
4. Use **< Prev**, **Next >**, or **Jump** to replay solution steps
5. Click **Save** to save result to `test/output/<name>.txt`

## Map File Format

- Line 1: `<rows> <cols>`
- Lines 2+: Grid (`X` wall, `*` ice, `Z` start, `O` goal, `L` lava, `0`-`9` sequence)
- Then: Cost matrix (same dimensions as grid)

## Author
|        Nama         |    NIM   |
|---------------------|----------|
| Nathaniel Christian | 13524122 |
| Ahmad Fauzan Putra  | 13524141 |