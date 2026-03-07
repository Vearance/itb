# Tugas Kecil 1 Stima


Algoritma penyelesaian permainan Queens dengan menggunakan algoritma brute-force, ditulis dalam Go. Permainan Queens dimainkan pada sebuah papan berukuran N x N yang terdiri dari N warna berbeda.

Aturan: satu queen per baris, per kolom, dan per warna. Selain itu, tidak boleh ada queen yang tepat bersebelahan (beserta diagonal).

Program akan menerima board kosong dengan konfigurasi warna tertentu.

## Compile

1. Clone repository ini:
   ```bash
   git clone https://github.com/Vearance/Tucil1_13524122.git
   cd Tucil1_13524122
   ```
   *NOTE*: Clone repository ini penting, karena test case akan disimpan dalam `test/` dan hasil save akan ada di dalam `test/solution/`. Tanpa folder ini, pembacaan file dan save file akan gagal.

2. Kompilasi program:
   ```bash
   cd src
   go build -o ../bin/solver
   ```
   Executable akan tersimpan di folder `bin/`.

Alternatif: jalankan langsung tanpa compile (lihat bagian berikutnya).

## Cara Menjalankan Program

### Opsi 1: Jalankan langsung dengan `go run`

```bash
cd src
go run .
```

### Opsi 2: Jalankan executable hasil compile

```bash
cd bin
./solver
```

### Penggunaan

1. Program akan meminta nama file input. Letakkan file test case di dalam folder `test/`, lalu masukkan nama file-nya (contoh: `case-1.txt`).

   ```
   Input nama file (no whitespace, put inside test/ folder): case-1.txt
   ```

2. Pilih algoritma mana yang ingin digunakan. Apabila ingin menggunakan pure brute force (exhaustive), input `1`. Sebaliknya apabila ingin menggunakan brute force efisien dengan backtracking, input `2`.

   ```
   Pakai algoritma exhaustive (1) atau backtracking (2)? Input 1 atau 2: 2
   ```

3. Program akan menampilkan proses pencarian di terminal, kemudian menampilkan waktu pencarian dan jumlah kasus yang ditinjau.

4. Program akan menanyakan apakah solusi ingin disimpan ke file:
   ```
   Apakah Anda ingin menyimpan solusi? (Y/N): Y
   ```
   Jika `Y`, solusi akan disimpan di `test/solution/<nama_file>`. Nama file akan sesuai dengan nama file input.

### Format File Input

File input berupa grid karakter N x N di mana setiap karakter merepresentasikan sebuah daerah warna. Contoh (`test/case-1.txt`):

```
AAABBCCCD
ABBBBCECD
ABBBDCECD
AAABDCCCD
BBBBDDDDD
FGGGDDHDD
FGIGDDHDD
FGIGDDHDD
FGGGDDHHH
```

Di file output, queen yang berhasil ditempatkan ditandai dengan karakter `#`.

## Author

| NIM | Nama |
|---|---|
| 13524122 | Nathaniel Christian |

_Disclaimer: Baru belajar Golang, jadi kalau ada struktur/convention yang kurang mohon dimaklumi :D._