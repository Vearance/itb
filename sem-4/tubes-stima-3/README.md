# Tubes3_lintasjava

Chromium browser extension untuk mendeteksi konten judi online pada halaman web menggunakan algoritma Knuth-Morris-Pratt, Boyer-Moore, Aho-Corasick, Rabin-Karp, dan RegEx dengan dukungan fuzzy matching berbasis Weighted Levenshtein Distance, OCR gambar menggunakan Tesseract.js, censorship pada elemen terdeteksi, dan visualisasi hasil & statistik performa algoritma.

## Daftar Isi
- [Deskripsi Tugas Besar](#deskripsi-tugas-besar)
- [Penjelasan Singkat Algoritma KMP dan BM](#penjelasan-singkat-algoritma-kmp-dan-bm)
- [Checklist Fitur](#checklist-fitur)
- [Struktur Project](#struktur-project)
- [Prasyarat Program dan Panduan Instalasi](#prasyarat-program-dan-panduan-instalasi)
- [Panduan Build Extension](#panduan-build-extension)
- [Panduan Memuat Extension di Chrome](#panduan-memuat-extension-di-chrome)
- [Kontributor](#kontributor)

## Deskripsi Tugas Besar
Tugas Besar 3 Strategi Algoritma bertujuan untuk mengimplementasikan algoritma *pattern matching* pada sebuah *chromium browser extension* untuk mendeteksi konten judi online pada halaman web. Program melakukan proses pemindaian terhadap elemen DOM pada halaman web, kemudian mencocokkan teks menggunakan algoritma Knuth-Morris-Pratt (KMP), Boyer-Moore(BM), dan Regular Expression (RegEx). Selain *exact matching*, sistem menerapkan *fuzzy matching* menggunakan Weighted Levenshtein Distance untuk mendeteksi manipulasi karakter pada kata kunci judi online. Program juga mendukung fitur *censorship* berupa blur pada elemen yang terdeteksi, dan *optical character recognition (OCR)* menggunakan Tesseract.js untuk mendeteksi teks pada gambar. Hasil ditampilkan sebagai *highlight* pada elemen DOM, *tooltip* informasi hasil pencocokkan, dan statistik performa algoritma secara *real-time* pada *popup extension*. Implementasi dilakukan menggunakan bahasa pemrograman TypeScript.

## Penjelasan Singkat Algoritma KMP dan BM

*   **Algoritma Knuth-Morris-Pratt (KMP)**

Algoritma *exact string matching* yang meminimalkan perbandingan karakter dengan memanfaatkan informasi pergeseran (*partial match* atau *fail function*) yang dihitung sebelumnya dari pola pencarian. Jika terjadi ketidakcocokan, algoritma dapat melompati perbandingan karakter yang redundan secara efisien.

*   **Algoritma Boyer-Moore (BM)**

Algoritma *exact string matching* yang sangat efisien dengan memindai karakter pola dari kanan ke kiri. Algoritma ini menggunakan dua aturan pergeseran untuk melompati beberapa karakter teks sekaligus, yaitu *Bad Character Rule* dan *Good Suffix Rule*.  *Bad Character Rule* mencocokkan karakter yang salah dengan kecocokan terakhir di pola. Sedangkan *Good Suffix Rule* mencari kecocokan substring pola yang cocok dengan akhiran sebelumnya.

## Checklist Fitur
| No | Poin | Ya | Tidak |
|----|------|----|-------|
| 1 | Extension berhasil di-build dan di-load tanpa kesalahan pada chromium browser dan dikembangkan dengan TypeScript | ✓ |  |
| 2 | KMP dan Boyer-Moore diimplementasikan from scratch | ✓ |  |
| 3 | Regex menghandle format <kata><angka> dan berbagai edge case | ✓ |  |
| 4 | Pencarian KMP & BM membaca keyword.txt secara iteratif dan tidak menggunakan built-in search function atau library eksternal | ✓ |  |
| 5 | Exact matching dan fuzzy matching berjalan benar | ✓ |  |
| 6 | Elemen DOM terdeteksi diberi highlight dan terhapus saat rescanning | ✓ |  |
| 7 | Tooltip muncul saat hover dengan informasi keyword, Algoritma, kemunculan, dan waktu eksekusi | ✓ |  |
| 8 | Popup menampilkan statistik realtime (total keyword, perbandingan, waktu eksekusi, jumlah match) | ✓ |  |
| 9 | [Bonus] Membuat video | ✓ |  |
| 10 | [Bonus] Implementasi Algoritma Aho-Corasick dan Rabin Karp | ✓ |  |
| 11 | [Bonus] Implementasi Censorship / Blur Teks | ✓ |  |
| 12 | [Bonus] Implementasi Optical Character Recognition pada Gambar | ✓ |  |

## Struktur Project
```
Tubes3_lintasjava
├── dist/
├── doc/                        
├── keywords/                   
├── public/                    
├── src/ 
│   ├── algorithms/
│   │   ├── ahoCorasick.ts
│   │   ├── bm.ts
│   │   ├── kmp.ts
│   │   ├── rabinKarp.ts       
│   │   ├── regex.ts
│   │   └── weightedLevenshteinDistance.ts
│   ├── content/               
│   │   ├── constants.ts       
│   │   ├── content.ts         
│   │   ├── domScanner.ts      
│   │   ├── highlighter.ts
│   │   ├── homoglyphs.ts
│   │   ├── imageDetection.ts
│   │   ├── imageReplacement.ts
│   │   ├── matchers.ts        
│   │   ├── ocrEngine.ts
│   │   ├── statistics.ts      
│   │   ├── storage.ts         
│   │   ├── tooltip.ts         
│   │   ├── types.ts
│   │   └── unicode.ts
│   └── popup/               
│       ├── popup.html        
│       ├── popup.ts           
│       └── visualization.ts   
├── package.json              
├── tsconfig.json              
└── vite.config.ts             
```

## Prasyarat Program dan Panduan Instalasi
Sebelum menjalankan program, pastikan perangkat Anda telah terpasang:
*   [Node.js](https://nodejs.org/) (Versi LTS sangat disarankan)
*   [npm](https://www.npmjs.com/) (Biasanya otomatis terinstal bersama Node.js)
*   Browser berbasis Chromium seperti Google Chrome, Microsoft Edge, Brave, atau Opera.

## Panduan Build Extension
1. Clone repository dengan menjalankan perintah:
```bash
git clone https://github.com/Irvin-Tandiarrang-Sumual/Tubes3_lintasjava.git
```
atau unduh zip proyek ini dan extract folder

2. Masuk ke direktori proyek dengan menjalankan perintah:
```bash
cd Tubes3_lintasjava
```

3. Instal seluruh dependensi yang diperlukan dengan menjalankan perintah:
```bash
npm install
```

4. Lakukan kompilasi dan bundling proyek menggunakan Vite dengan perintah:
```bash
npm run build
```

5. Hasil kompilasi yang siap dipasang akan berada di dalam direktori `/dist`

## Panduan Memuat Extension di Chrome
1. Buka browser berbasis Chromium, seperti Google Chrome, Microsoft Edge, Brave, atau Opera
2. Akses halaman pengelolaan ekstensi dengan mengetik `chrome://extensions/` pada address bar
3. Aktifkan **Developer mode** yang berada di pojok kanan atas halaman
4. Klik tombol **Load unpacked** yang berada di pojok kiri atas halaman
5. Pilih folder `/dist` yang berada di dalam direktori proyek ini
6. Ekstensi **Judol Detector** sekarang telah terpasang dan akan aktif memindai halaman web secara otomatis

## Kontributor
| NIM      | Nama                           |
|:---------|:-------------------------------|
| 13524030 | Irvin Tandiarrang Sumual       | 
| 13524089 | Aurelia Jennifer Gunawan       |
| 13524122 | Nathaniel Christian            |
