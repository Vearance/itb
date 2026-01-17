module Mahasiswa where

-- TYPE MAHASISWA
-- DEFINISI TYPE
-- type mahasiswa: <nama: String, nim: String, ipk: Float>
-- {<nama, nim, ipk> adalah sebuah mahasiswa, dengan nama adalah nama mahasiswa,
--  nim adalah nomor induk mahasiswa, ipk adalah nilai IPK mahasiswa}

data Mahasiswa = Mahasiswa String String Float
  deriving (Show, Eq)

-- DEFINISI DAN SPESIFIKASI SELEKTOR
-- nama: mahasiswa -> String
-- {nama(M) memberikan nama Mahasiswa M}
nama :: Mahasiswa -> String
nama (Mahasiswa m _ _) = m

-- nim: mahasiswa -> String
-- {nim(M) memberikan NIM Mahasiswa M}
nim :: Mahasiswa -> String
nim (Mahasiswa _ n _) = n

-- ipk: mahasiswa -> Float
-- {ipk(M) memberikan IPK Mahasiswa M}
ipk :: Mahasiswa -> Float
ipk (Mahasiswa _ _ i) = i

-- DEFINISI DAN SPESIFIKASI KONSTRUKTOR
-- makeMahasiswa: String -> String -> Float -> mahasiswa
-- {makeMahasiswa(nama,nim,ipk) membentuk Mahasiswa baru}
makeMahasiswa :: String -> String -> Float -> Mahasiswa
makeMahasiswa m n i = Mahasiswa m n i

-- DEFINISI DAN SPESIFIKASI PREDIKAT
-- isValidMahasiswa: mahasiswa -> boolean
-- {isValidMahasiswa(M) benar jika M memiliki Nama, NIM tidak kosong dan IPK antara 0.0 s.d. 4.0}
isValidMahasiswa :: Mahasiswa -> Bool
isValidMahasiswa mhs
    | (nama mhs /= "") && (nim mhs /= "") && (ipk mhs >= 0) && (ipk mhs <= 4) = True
    | otherwise = False

-- DEFINISI OPERATOR/FUNGSI LAIN TERHADAP MAHASISWA
-- gantiIPK: mahasiswa -> Float -> mahasiswa
-- {gantiIPK(M,ipkBaru) mengganti IPK mahasiswa M dengan ipkBaru (maksimal 4.0)}
gantiIPK :: Mahasiswa -> Float -> Mahasiswa
gantiIPK mhs i
    | i > 4 = makeMahasiswa (nama mhs) (nim mhs) (4)
    | i < 0 = makeMahasiswa (nama mhs) (nim mhs) (0)
    | otherwise = makeMahasiswa (nama mhs) (nim mhs) (i)

-- klasifikasiMahasiswa: mahasiswa -> String
-- {klasifikasiMahasiswa(M) memberikan predikat mahasiswa berdasarkan IPK}
-- Dengan ketentuan:
-- IPK >= 3.51 : "Cumlaude"
-- 3.00 <= IPK < 3.51 : "Sangat Memuaskan"
-- 2.75 <= IPK < 3.00 : "Memuaskan"
-- IPK < 2.75 : "Perlu Perbaikan"
klasifikasiMahasiswa :: Mahasiswa -> String
klasifikasiMahasiswa mhs
    | (ipk mhs >= 3.51) && (ipk mhs <= 4) = "Cumlaude"
    | (ipk mhs >= 3) && (ipk mhs < 3.51) = "Sangat Memuaskan"
    | (ipk mhs >= 2.75) && (ipk mhs < 3) = "Memuaskan"
    | (ipk mhs < 2.75) && (ipk mhs >= 0) = "Perlu Perbaikan"
    | otherwise = "Perlu Perbaikan"

-- tampilMahasiswa: mahasiswa -> String
-- {tampilMahasiswa(M) mengubah Mahasiswa M menjadi string deskriptif}
-- Dengan format:
-- "Nama: <nama>, NIM: <nim>, IPK: <ipk>"
tampilMahasiswa :: Mahasiswa -> String
tampilMahasiswa mhs =
    "Nama: " ++ (nama mhs) ++ ", NIM: " ++ (nim mhs) ++ ", IPK: "
    ++ show(ipk mhs)