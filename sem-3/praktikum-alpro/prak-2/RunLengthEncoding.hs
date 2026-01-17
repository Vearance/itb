module RunLengthEncoding where

runLengthEncoding :: String -> [(Char, Int)]
runLengthEncoding l
  | null l = []
  | otherwise = (head l, countSame (head l) l) : runLengthEncoding (delete (head l) l)

countSame :: Char -> String -> Int
countSame c l
  | null l = 0
  | head l == c = 1 + countSame c (tail l)
  | otherwise = 0

delete :: Char -> String -> String
delete c l
  | null l = []
  | head l == c = delete c (tail l)
  | otherwise = l
