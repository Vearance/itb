module ColumnOperations where 

-- Lengkapilah realisasi fungsi ColumnOperations dari file ColumnOperations.hs yang menerima masukan sebuah list of list (Matriks) Integer dan akan melakukan operasi reduce untuk setiap kolom yang ada di dalam matriks tersebut. Untuk setiap kolom operasi yang akan dilakukan adalah pencarian nilai maksimum kolom pertama, lalu pencarian nilai minimum untuk kolom selanjutnya, dan diikuti perhitungan jumlah untuk kolom berikutnya. Operasi ini akan dilakukan secara berulang - ulang dan sebagai gambaran, operasi yang dilakukan adalah max, min, sum, max, min, sum, dst.

-- - Matriks bisa kosong atau memiliki list - list yang kosong, contoh: [], [[],[],[]]

-- > columnOperations [[1,2,3],[4,5,6],[7,8,9]]

-- [7,2,18]

-- Keterangan: 7 adalah nilai maksimum kolom pertama (1, 4, 7), 2 adalah minimum kolom kedua (2, 5, 8), dan 18 adalah jumlah elemen - elemen di kolom ketiga (3, 6, 9).

maxList :: [Int] -> Int
maxList l =
    if null (tail l) then head l
    else
        let m = maxList (tail l)
        in if head l > m then head l else m

minList :: [Int] -> Int
minList l =
    if null (tail l) then head l
    else
        let m = minList (tail l)
        in if head l < m then head l else m

sumList :: [Int] -> Int
sumList l =
    if null l then 0
    else head l + sumList (tail l)


konso :: a -> [a] -> [a]
konso e l = e : l

konsdot :: [a] -> a -> [a]
konsdot l e = l ++ [e]


isEmptyInt :: [Int] -> Bool
isEmptyInt l = null l

isEmptyList :: [[Int]] -> Bool
isEmptyList l = null l

isEmpty :: [a] -> Bool
isEmpty l = null l



firstCol :: [[Int]] -> [Int]
firstCol l =
    if isEmpty l then []
    else
        if isEmpty (head l) then []
        else konso (head (head l)) (firstCol (tail l))


reCols :: [[Int]] -> [[Int]]
reCols l =
    if isEmpty l then []
    else
        if isEmpty (head l) then []
        else konso (tail (head l)) (reCols (tail l))


columnOperations :: [[Int]] -> [Int]
columnOperations l = 
    columnOps l 0
    where
        columnOps m step =
            if isEmpty m || isEmpty (head m) then []
            else
                let col = firstCol m
                    rest = reCols m
                    val = if (mod step 3) == 0 then maxList col
                          else if (mod step 3) == 1 then minList col
                          else sumList col
                in konso val (columnOps rest (step+1))