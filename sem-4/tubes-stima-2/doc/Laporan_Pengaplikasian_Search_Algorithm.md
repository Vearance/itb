# Laporan Pengaplikasian Algoritma BFS dan DFS pada DOM Tree Traversal

Aplikasi ini menggunakan algoritma **Breadth-First Search (BFS)** dan **Depth-First Search (DFS)** untuk melakukan simulasi proses *parsing* dan pencarian (*querying*) elemen dari sebuah struktur pohon DOM (Dokumen HTML) berdasarkan CSS Selector. 

Berikut adalah penjabaran pengaplikasian masing-masing algoritma berdasarkan baris instruksi kode sumber program.

## 1. Pengaplikasian BFS (Breadth-First Search)
*(diterapkan pada file `backend/services/search/BFS.go`)*

Algoritma BFS digunakan jika pengguna menginginkan pencarian yang menyebar secara hierarki, di mana elemen-elemen yang berada pada kedalaman (depth) yang lebih dangkal di pohon DOM akan dievaluasi terlebih dahulu sebelum elemen yang lebih dalam.

### Mekanisme Pengaplikasian di Kode:
1. **Penggunaan Struktur Antrean Bertingkat (Priority Queue):**
   BFS umumnya menggunakan murni FIFO (First-In First-Out) `Queue`. Namun, karena DOM traveral bisa memiliki lebih dari satu langkah selektor bertingkat (`selector.Steps`), queue diinisasikan berupa *slice/array dynamic* `[]QueueItem` yang disortir ulang setiap kali diiterasi menggunakan fungsi bawaan `sort.SliceStable()`.
2. **Prioritas Kedalaman:**
   Fungsi prioritas menempatkan kandidat elemen berdasarkan `Depth` node yang dituju (*level-by-level processing*), sehingga elemen yang paling dekat dengan *root* selalu diuji kemiripannya dengan selector sebelum sub-elemen anaknya.
3. **Ekspansi Selektor Lanjutan:**
   Saat mengevaluasi sebuah elemen kandidat, jika elemen tersebut cocok dengan *selector step* saat ini namun memiliki sambungan penelusuran lebih lanjut, BFS mencatat elemen-elemen kerabat/anaknya (*RelatedNodes*) dan memasukkannya lagi ke proses antrean untuk `stepIndex+1` tanpa merusak urutan kedalaman sisa pohon utama.

## 2. Pengaplikasian DFS (Depth-First Search) dengan *Multithreading*
*(diterapkan pada file `backend/services/search/DFS.go`)*

Algoritma DFS diaplikasikan untuk penelusuran menyusur kedalaman elemen secara rekursif terlebih dahulu hingga daun (ujung terdalam child DOM), sebelum menelusuri saudaranya. Algoritma tipe ini secara arsitektur lebih rapi dan sangat optimal untuk diaplikasikan ke sistem **Multithreading** karena setiap cabang (branch) dapat diproses terpisah dan tanpa membutuhkan sebuah antrean terpusat yang ketat. 

### Mekanisme Pengaplikasian di Kode:
1. **Goroutine Worker per Cabang (Fork-Join Model Parallelism):**
   Dalam fungsi induk `traverse`, setiap iterasi dari *child nodes* akan dipecah dengan membangkitkan `Goroutine` baru (`go func(...)`). Cara ini akan menciptakan "thread/worker ringan" tersendiri bagi setiap cabang pohon, memungkinkan eksplorasi simultan untuk node-node yang berdampingan di keseluruhan cabang.
2. **Sinkronisasi Thread (Wait Groups):**
   Karena pemanggilan rekursifnya bercabang sangat banyak, aplikasi mengikat thread-thread yang menyebar tersebut menggunakan `sync.WaitGroup` (`wg.Add(1)` dan `wg.Done()`). Ini mencegah eksekusi program utamanya melompat selesai sebelum seluruh turunan thread pelacakan tuntas.
3. **Keamanan Mutex (Data Race Prevention):**
   Untuk mewujudkan fungsi Multithreading yang kokoh dan bebas dari galat kolisi memori, operasi yang membutuhkan perubahan data secara kolektif—seperti penambahan node yang ketemu sasaran (`results.NodeIDs`) atau penulisan log perjalanan node yang difilter (`log.UpsertLogEntry`)—dilindungi dengan pintu gembok (*lock mechanism*) tunggal menggunakan `sync.Mutex`.
4. **Pembatasan Atomic (Early Constraints Exit):**
   Jika pengguna telah membatasi pencarian dengan parameter batas pencarian ekivalen `amount`, maka pengecekan penghentian tidak bisa dilakukan perulangan standar. Parameter batas dirancang mendasarkan `stop *atomic.Bool` dari package `sync/atomic` yang bertugas menjadi saklar darurat. Seketika target jumlah nodenya terpenuhi di *thread* A, ia akan menjadikannya `true`, dan 100 *thread* cabang lainnya di *thread* B/C/D akan seketika itu juga memutus perjalanannya pada awal fungsinya secara sinkronis.
