module Matrix where 

-- UTILITY FUNCTIONS
konsdot :: [Int] -> Int -> [Int]
konso :: Int -> [Int] -> [Int]
isEmpty :: [Int] -> Bool

konso e l = [e] ++ l
konsdot l e = l ++ [e]
isEmpty l = null l

konsdotMatrix :: [[Int]] -> [Int] -> [[Int]]
konsoMatrix :: [Int] -> [[Int]] -> [[Int]]
isEmptyMatrix :: [[Int]] -> Bool

konsoMatrix e l = [e] ++ l
konsdotMatrix l e = l ++ [e]
isEmptyMatrix l = null l

-- NOTES: Semua Index dipastikan valid, yaitu tidak negatif dan tidak melebihi panjang list/Matrix
-- MATRIX MANIPULATION FUNCTIONS

-- TYPE: Matrix adalah [[Int]]
type Matrix = [[Int]]

addAtIndex :: Int -> [Int] -> Int -> [Int]
addAtIndex x l i =
    if isEmpty l then [x]
    else if i == 0 then konso x l
    else konso (head l) (addAtIndex x (tail l) (i-1))

deleteAtIndex :: [Int] -> Int -> [Int]
deleteAtIndex l i =
    if isEmpty l then []
    else if i == 0 then tail l
    else konso (head l) (deleteAtIndex (tail l) (i-1))

editAtIndex :: [Int] -> Int -> Int -> [Int]
editAtIndex l i newValue =
    if isEmpty l then []
    else if i == 0 then konso newValue (tail l)
    else konso (head l) (editAtIndex (tail l) (i-1) newValue)

addMatrixElement :: Matrix -> Int -> Int -> Int -> Matrix
addMatrixElement m row col value =
    if isEmptyMatrix m then []
    else if row == 0 then
        konsoMatrix (addAtIndex value (head m) col) (tail m)
    else
        konsoMatrix (head m) (addMatrixElement (tail m) (row-1) col value)

deleteMatrixElement :: Matrix -> Int -> Int -> Matrix
deleteMatrixElement m row col =
    if isEmptyMatrix m then []
    else if row == 0 then
        konsoMatrix (deleteAtIndex (head m) col) (tail m)
    else
        konsoMatrix (head m) (deleteMatrixElement (tail m) (row-1) col)

editMatrixElement :: Matrix -> Int -> Int -> Int -> Matrix
editMatrixElement m row col newValue =
    if isEmptyMatrix m then []
    else if row == 0 then
        konsoMatrix (editAtIndex (head m) col newValue) (tail m)
    else
        konsoMatrix (head m) (editMatrixElement (tail m) (row-1) col newValue)

getMatrixElement :: Matrix -> Int -> Int -> Int
getMatrixElement m row col =
    if isEmptyMatrix m then 0
    else if row == 0 then
        if isEmpty (head m) then 0
        else if col < length (head m) then head (drop col (head m)) else 0
    else getMatrixElement (tail m) (row-1) col

printMatrix :: Matrix -> IO ()
printMatrix [] = putStrLn "Empty m"
printMatrix m = mapM_ print m
