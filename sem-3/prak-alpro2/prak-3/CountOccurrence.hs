module CountOccurrence where

-- count(ListOfList, n) menghitung berapa kali integer n muncul di dalam sebuah list of list.
-- CONTOH: count [[1,2,1],[3],[1,4]] 1 menghasilkan 3
count :: [[Int]] -> Int -> Int
-- TODO : implementasi fungsi count 
count l n =
    if null l then 0
    else
        if null (head l) then count (tail l) n
        else
            let
                m = head l
                k = tail l 
            in
                if head(m) == n then 1 + count ([tail(m)] ++ k) n
                else
                    count ([tail(m)] ++ k) n

-- rmvMem :: Int -> [[Int]] -> [[Int]]
-- rmvMem a l =
--     if null l == True then l
--     else
        

-- count :: [[Int]] -> Int -> Int
-- -- TODO : implementasi fungsi count 
-- count l n
--     | null l == True = 0
--     | null (head l) == True = count (tail l) n
--     | head(head l) == n = 1 + count ((head(tail l)) n)
--     | otherwise = count (head(tail l)) n
