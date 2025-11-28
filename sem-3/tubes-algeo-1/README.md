# Aljabar Linier dan Geometri Tubes 1
| NIM      | Nama                     |
|:---------|:-------------------------|
| 13524030 | Irvin Tandiarrang Sumual |
| 13524089 | Aurelia Jennifer Gunawan |
| 13524122 | Nathaniel Christian      |


## Penjelasan Singkat Program
Template ini merupakan project Java untuk Tubes 1 Algeo 2025/2026. Project menggunakan Maven dan dapat dijalankan sebagai GUI (`JavaFX`).  


## Alur Program
1. User menjalankan program via GUI.
2. User memilih salah satu dari lima fungsi utama program.
3. Program membaca input dalam bentuk teks maupun langsung import dalam bentuk file.
4. Hasil perhitungan ditampilkan ke window JavaFX. Diberikan juga langkah-langkah penyelesaian apabila memungkinkan.
5. User dapat melakukan export atau save file terhadap output.

## Tata Cara Menjalankan Program

Perintah berikut akan menghasilkan folder `target` yang berisi `file matrix-calculator.jar`
```bash
mvn clean package
```

Sebagai alternatif, jika kamu tidak ingin membuat file .jar, kamu bisa menggunakan
```bash
mvn clean compile
```


Untuk run GUI, gunakan:
```bash
mvn exec:java
```
atau
```bash
mvn clean javafx:run
```

## Requirements

### Java
- **Version:** 17 atau lebih tinggi  
- [Oracle JDK 17 Download](https://www.oracle.com/java/technologies/downloads)

### Maven
- **Version:** 3.2.5 atau lebih tinggi (direkomendasikan 3.6.3+)  
- [Apache Maven Download](https://dlcdn.apache.org/maven/maven-3/3.9.11/binaries/apache-maven-3.9.11-bin.zip)

### Tambahan
- Windows: Pastikan folder `bin` dari JDK & Maven ada di PATH.
- Linux / MacOS: Install via apt/brew sesuai instruksi di template.
