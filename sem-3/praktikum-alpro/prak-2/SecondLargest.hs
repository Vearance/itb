module SecondLargest where

secondLargest :: [Int] -> Int
secondLargest l =
    maxv(removeMax(l))
    
removeMax :: [Int] -> [Int]
removeMax [] = []
removeMax (x:xs)
    | x == m = xs
    | otherwise = x : removeMax xs
    where
    m = maxv (x:xs)

maxv :: [Int] -> Int
maxv l =
    if (length l == 1) then
        head l
    else
        let
            m = maxv (tail l)
        in
            if (m < head l) then
                head l
            else
                m