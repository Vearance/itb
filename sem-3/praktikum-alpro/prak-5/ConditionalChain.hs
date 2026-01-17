module ConditionalChain where

-- Deskripsi:
-- Fungsi `conditionalChain` menerima sebuah pasangan aturan yang terdiri dari
-- kondisi dan fungsi transformasi, serta sebuah list (bisa kosong).
-- Setiap elemen pada list akan dicek terhadap kondisi tersebut.
-- Jika elemen memenuhi kondisi, maka elemen tersebut akan diubah
-- menggunakan fungsi transformasi. Jika tidak, elemen dibiarkan tetap.

-- Contoh:
-- conditionalChain ((\x -> x < 0), (\x -> x - 1)) [-2, 0, 4] => [-3, 0, 4]

konso:: a -> [a] -> [a]
konso e l = e : l

isEmpty:: [a] -> Bool
isEmpty l = null l

conditionalChain :: (a -> Bool, a -> a) -> [a] -> [a]
conditionalChain f l =
    if isEmpty l then []
    else 
        if (fst f (head l)) then konso (snd f (head l)) (conditionalChain f (tail l))
        else konso (head l) (conditionalChain f (tail l))
-- TODO
-- Note: Dilarang menggunakan map