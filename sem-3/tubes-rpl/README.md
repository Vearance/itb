# Tabungin - Aplikasi Manajemen Keuangan Pribadi

## Anggota Tim

| NIM | Nama | Kontribusi |
|-----|------|-----------|
| 13524120 | Jonathan Alveraldo Bangun | Frontend (TambahRekening, TambahTransaksi, TambahTarget) |
| 13524122 | Nathaniel Christian | Backend (most of model-controller), Frontend, connect backend-frontend |
| 13524125 | Muhammad Rafi Akbar | Frontend (ProgressView, HistoryView, LaporanView, RekeningView, TambahRekening, TambahTransaksi, TambahTarget); UI Design | 
| 13524139 | Azri Arzaq Pohan | Backend (Transaction, Account), Frontend (Loginview, MainWindow, Homepage, RekeningView) |
| 13524142 | Rasyad Satyatma | connect backend-frontend, Backend (Target), Frontend (TargetView, graph components, etc.) |

## Daftar Isi
- [Penjelasan Aplikasi](#penjelasan-aplikasi)
- [Instalasi](#instalasi)
- [Menjalankan Aplikasi](#menjalankan-aplikasi)
- [Daftar Modul yang Diimplementasi](#daftar-modul-yang-diimplementasi)
- [Daftar Database](#daftar-database)

## Penjelasan Aplikasi

**Tabungin** adalah aplikasi manajemen keuangan pribadi yang dirancang untuk membantu pengguna mengelola keuangan mereka dengan mudah dan efisien. Aplikasi ini menyediakan fitur-fitur untuk:

- **Manajemen Rekening**: Membuat dan mengelola beberapa rekening/tabungan dengan saldo terpisah
- **Pencatatan Transaksi**: Mencatat pemasukan dan pengeluaran untuk setiap rekening
- **Target Tabungan**: Menetapkan target tabungan dengan deadline dan melacak progresnya
- **Laporan Keuangan**: Melihat laporan terperinci tentang riwayat transaksi dan perkembangan target
- **Keamanan Akun**: Sistem login dengan password yang terenkripsi untuk keamanan data pengguna

Aplikasi ini dibangun dengan Python menggunakan arsitektur MVC (Model-View-Controller) dan menggunakan arsitektur Repository (SQLite) sebagai database.

## Instalasi

1. **Clone Repository**
   ```bash
   git clone https://github.com/AzriVz/IF2150-2025-K03-G03-Tabungin
   cd IF2150-2025-K03-G03-Tabungin
   ```

2. **Virtual Environment**
   ```bash
   python -m venv venv
   source venv/bin/activate  # Untuk Linux/macOS
   # atau
   venv\Scripts\activate  # Untuk Windows
   ```

3. **Install Dependencies**
   ```bash
   pip install -r requirements.txt
   ```

## Menjalankan Aplikasi

```bash
python src/main.py
```

Aplikasi akan membuka GUI utama dan secara otomatis membuat/menginisialisasi database `tabungin.db` jika belum ada (db bersifat lokal, jadi di exclude dari repo).


## Daftar Modul yang Diimplementasi

Modul dalam aplikasi Tabungin mengikuti pola arsitektur **Model-View-Controller (MVC) + Repository Pattern** sebagai berikut:

### 1. **View / UI Module** (Tampilan)
   - **Files**: `views/LoginView.py`, `views/MainWindow.py`, `views/Homepage.py`, `views/RekeningView.py`, `views/TambahRekening.py`, `views/HistoryView.py`, `views/TambahTransaksi.py`, `views/TargetView.py`, `views/TambahTarget.py`, `views/LaporanView.py`
   - **Deskripsi**: Semua menu atau tampilan yang dilihat oleh pengguna
   - **Fitur Utama**:
     - Login/Register
     - Dashboard/Homepage
     - Rekening List dan Form Tambah Rekening
     - Transaction History dan Form Tambah Transaksi
     - Target View dan Form Tambah Target
     - Report/Laporan View

### 2. **Controller Module** (Proses)
   - **Files**: `controllers/LoginController.py`, `controllers/MainController.py`, `controllers/AccountController.py`, `controllers/TransactionController.py`, `controllers/TargetController.py`, `controllers/ReportController.py`
   - **Deskripsi**: Mengatur logika aplikasi, memanggil utility atau repository jika diperlukan
   - **Fitur Utama**:
     - Mengelola proses login dan registrasi
     - Mengelola operasi rekening
     - Mengelola operasi transaksi
     - Mengelola operasi target
     - Mengelola laporan keuangan
     - Validasi input dari UI

### 3. **Model / Entity Module** (Model Data)
   - **Files**: `models/User.py`, `models/Account.py`, `models/Transaction.py`, `models/Target.py`
   - **Deskripsi**: Berisi struktur data dan relasi antar entity
   - **Fitur Utama**:
     - `User`: Struktur data pengguna
     - `Account`: Struktur data rekening dengan operasi saldo
     - `Transaction`: Struktur data transaksi (income/expense)
     - `Target`: Struktur data target tabungan dengan tracking progress

### 4. **Repository Module** (Penyimpanan Data)
   - **File**: `database.py`
   - **Deskripsi**: Melakukan operasi read/write ke dalam database, sebagai tempat penyimpanan utama data
   - **Fitur Utama**:
     - Koneksi dan manajemen database SQLite
     - Operasi CRUD (Create, Read, Update, Delete)
     - Inisialisasi tabel database
     - Query data dari database

### 5. **Security**
   - **File**: `utility/Security.py`
   - **Deskripsi**: Melakukan hashing/enkripsi untuk password pengguna
   - **Fitur Utama**:
     - Hashing password

## Daftar Database

Aplikasi ini menggunakan 4 tabel utama dalam SQLite database:

### 1. **Tabel: `users`**
   Menyimpan data pengguna yang terdaftar di aplikasi

   | Atribut | Tipe Data | Deskripsi | Constraint |
   |---------|-----------|-----------|-----------|
   | `user_id` | STRING | Identifikasi unik pengguna | PRIMARY KEY |
   | `username` | STRING | Nama pengguna untuk login | UNIQUE NOT NULL |
   | `password` | STRING | Password yang terenkripsi | NOT NULL |


### 2. **Tabel: `accounts`**
   Menyimpan data rekening/tabungan yang dimiliki pengguna

   | Atribut | Tipe Data | Deskripsi | Constraint |
   |---------|-----------|-----------|-----------|
   | `account_id` | STRING | Identifikasi unik rekening | PRIMARY KEY |
   | `account_name` | STRING | Nama rekening (misal: "Tabungan Umum", "Cicilan Rumah") | NOT NULL |
   | `user_id` | STRING | ID pengguna pemilik rekening | FOREIGN KEY references users(user_id) |
   | `balance` | REAL | Saldo saat ini dalam rekening | DEFAULT 0.0 |
   | `created_at` | DATETIME | Tanggal dan waktu pembuatan rekening | DEFAULT CURRENT_TIMESTAMP |


### 3. **Tabel: `transactions`**
   Menyimpan riwayat semua transaksi (pemasukan dan pengeluaran)

   | Atribut | Tipe Data | Deskripsi | Constraint |
   |---------|-----------|-----------|-----------|
   | `transaction_id` | STRING | Identifikasi unik transaksi | PRIMARY KEY |
   | `account_id` | STRING | ID rekening tempat transaksi terjadi | FOREIGN KEY references accounts(account_id) |
   | `amount` | REAL | Jumlah uang dalam transaksi | NOT NULL |
   | `type` | STRING | Jenis transaksi ("income" atau "expense") | NOT NULL |
   | `description` | STRING | Deskripsi atau catatan transaksi | |
   | `date` | DATETIME | Tanggal dan waktu transaksi | DEFAULT CURRENT_TIMESTAMP |


### 4. **Tabel: `targets`**
   Menyimpan data target tabungan yang ditetapkan pengguna

   | Atribut | Tipe Data | Deskripsi | Constraint |
   |---------|-----------|-----------|-----------|
   | `target_id` | STRING | Identifikasi unik target | PRIMARY KEY |
   | `account_id` | STRING | ID rekening tempat target disimpan | FOREIGN KEY references accounts(account_id) |
   | `target_name` | STRING | Nama target (misal: "Liburan Ke Bali") | NOT NULL |
   | `target_amount` | REAL | Jumlah target yang ingin dicapai | NOT NULL |
   | `current_amount` | REAL | Jumlah uang yang sudah terkumpul | DEFAULT 0.0 |
   | `is_achieved` | INTEGER | Status pencapaian target (0=belum, 1=tercapai) | DEFAULT 0 |
   | `is_archived` | INTEGER | Status archive target (0=aktif, 1=archived) | DEFAULT 0 |
   | `deadline` | TIMESTAMP | Batas waktu target harus dicapai | NOT NULL |
   | `created_at` | TIMESTAMP | Tanggal dan waktu pembuatan target | DEFAULT CURRENT_TIMESTAMP |





