# Tubes 1 Strategi Algoritma

### How to get started

Use [STIMA-battle client, mirrored from Battlecode 2025 Java Scaffold](https://github.com/Fariz36/STIMA-battle) to run the game. Copy the existing `src/` folder to the STIMA-battle client's `src/` folder.

### Useful Commands

- `./gradlew build`
    Compiles your player
- `./gradlew run`
    Runs a game with the settings in gradle.properties
- `./gradlew update`
    Update configurations for the latest version -- run this often
- `./gradlew zipForSubmit`
    Create a submittable zip file
- `./gradlew tasks`
    See what else you can do!

## Algoritma bitchlasagna
### 1. Tower
*   **Targeting:** Menyerang musuh terdekat dengan HP terendah, serang AoE jika banyak musuh.
*   **Spawn:** Melakukan spawn robot, dengan catatan sisa resources cukup untuk membuat tower baru

### 2. Soldier
*   **Retreat:** Kabur ke *Tower* terdekat jika HP/Cat di bawah 25%.
*   **Explore:** Mencari titik kosong. Saat menemukan *Ruin*, ia menggunakan fungsi *hash* koordinat untuk menentukan *tower type* untuk dibangun.

### 3. Mopper
*   **Patroli:** Bergerak ke petak dengan scoring: dekat cat teman (+10), dekat batas cat/kosong (+5), dekat teman sekarat (+50).
*   **Support:** Mengisi cat teman yang kritis, atau menyerang musuh cat terbanyak.

### 4. Splasher
*   **Targeting Ledakan:** Mencari target serangan dalam sense radius: kena tower musuh (+1000 point), kena robot musuh (+20), cat musuh (+5). Menembak titik dengan poin terbanyak.

## Algoritma holo_oven
### 1. Tower
*   **Targeting:** Menyerang musuh HP terendah, lalu menggunakan AoE jika musuh bergerombol.
*   **Spawn:** Ronde awal fokus spawn Soldier; setelah itu spawn rotasi 3:1:1 (Soldier:Mopper:Splasher) sambil menyisakan chip cadangan.

### 2. Soldier
*   **Retreat + Build:** Mundur saat HP/cat kritis, lalu memprioritaskan ruin untuk menandai, mengecat pola, dan menyelesaikan tower.
*   **Attack Mode:** Jika musuh terlihat, Soldier masuk mode serang dan menggunakan `pathfindAttack` agar langkah lebih agresif ke posisi tembak terbaik.

### 3. Mopper
*   **Support:** Mencari ally non-mopper dengan cat terendah untuk diisi ulang.
*   **Pressure:** Menyerang enemy dengan cat tertinggi dan bergerak ke perimeter (batas ally/empty/enemy paint) untuk menjaga area frontline.

### 4. Splasher
*   **Splash Targeting:** Menilai pusat ledakan terbaik dengan prioritas tower musuh, musuh, kontrol paint area.
*   **Movement:** Mengejar target musuh memakai `pathfindAttack`; jika tidak ada musuh terlihat, maju ke tengah map untuk mempercepat kontak.

## Algoritma weball
### 1. Tower
*   **Targeting:** Menyerang AoE jika terdapat musuh yang bergerombo, single attack jika hanya sedikit
*   **Spawn:** Melakukan spawn robot, dengan memprioritaskan Soldier dan Splasher

### 2. Soldier
*   **Attack:** Menyerang musuh jika menemukannya untuk mempertahankan teritori
*   **Explore:** Mencari tile kosong dan ruins untuk ekspansi map

### 3. Mopper
*   **Mop:** Menggunakan Mop jika berada di sekitar tile musuh
*   **Swing:** Menggunakan Swing jika terdapat musuh di sekitarnya
*   **Support:** Mengisi cat teman terdekat

### 4. Splasher
*   **Splash:** Mencari target serangan Splash yang memprioritaskan menyerang tower dan tile musuh, menghindari menimpa tile tim sendiri

---
\
**Kelompok AcademyOfFineArts**
| Nama | NIM |
| :--- | :--- |
| Bernhard Aprillio Pramana | 13524074 |
| Moreno Syawali Ganda Sugita | 13524096 |
| Nathaniel Christian | 13524122 |
