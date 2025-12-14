[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/WQhgEYKW)

# ITBooks

Aplikasi web untuk mencari buku berdasarkan **judul**, **konten dokumen**, atau **gambar cover** menggunakan algoritma **Latent Semantic Analysis (LSA)** dan **Principal Component Analysis (PCA)**.

## Anggota Kelompok

| NIM | Nama |
|-----|------|
| 13524122 | Nathaniel Christian |
| 13524139 | Azri Arzaq Pohan |
| 13524142 | Rasyad Satyatma |

## Fitur Utama

- **Pencarian berdasarkan Judul** - Substring matching untuk mencari buku berdasarkan judul
- **Pencarian berdasarkan Dokumen** - Upload file `.txt` untuk mencari buku dengan konten serupa menggunakan LSA
- **Pencarian berdasarkan Gambar** - Upload gambar cover untuk mencari buku dengan cover serupa menggunakan PCA/Eigencovers
- **Rekomendasi Buku** - Mendapatkan rekomendasi buku serupa berdasarkan buku yang dipilih


## Quick Start

```bash
# Install Git-LFS
git lfs install 

# Clone repository
git clone https://github.com/IRK-23/algeo2-ngikut-stress.git
cd algeo2-ngikut-stress
```

### Menggunakan Docker

```bash
# Jalankan dengan Docker Compose
docker compose up --build
```

Aplikasi akan tersedia di:
- Frontend: http://localhost:3000
- Backend API: http://localhost:8000

### Manual Setup

#### Backend

```bash
# Buat virtual environment
python -m venv venv

# Aktivasi virtual environment
# Windows:
venv\Scripts\activate
# Linux/Mac:
source venv/bin/activate

# Install dependencies
pip install -r src/backend/requirements.txt

# Jalankan backend
python src/backend/main.py
```

#### Frontend

```bash
cd src/frontend

# Install dependencies
npm install

# Jalankan development server
npm run dev
```

## Struktur Project

```
root
├── data/                   # Dataset dan cache
│   ├── covers/             # Gambar cover buku
│   ├── txt/                # Konten teks buku
│   ├── cache/              # Cache hasil preprocessing
│   └── mapper.json         # Metadata buku
├── docs/                   # Laporan dan dokumentasi
├── src/
│   ├── backend/            # FastAPI backend
│   │   ├── app/
│   │   │   ├── api/        # API routes
│   │   │   └── lib/        # Core algorithms (LSA, PCA)
│   │   └── main.py
│   └── frontend/           # Next.js frontend
│       ├── app/            # App router pages
│       └── components/     # React components
├── test/                   # Test cases
├── docker-compose.yml
└── README.md
```
