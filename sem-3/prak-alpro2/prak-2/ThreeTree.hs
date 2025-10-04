module ThreeTree where

threeTree :: [Int] -> [Int]
threeTree l
  | null l = []
  | mod (head l) 3 == 0 = [head l] ++ threeTree (tail l)
  | otherwise = threeTree (tail l)
