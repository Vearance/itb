# Automatic Grader

This repository contains a Python-based automatic grading system for checking students’ lab submissions.
Each student submits a .zip file containing Python scripts.

## Directory Structure

```text
.
├── grader.py              # Main grading script
├── submissions/           # ZIP files from students
│   ├── 16525113-1-P01_16525113.zip
│   └── ...
├── tests/                 # Test cases and expected outputs
│   ├── 01/
│   │   ├── test1.txt
│   │   ├── expected1.txt
│   │   ├── test2.txt
│   │   └── expected2.txt
│   ├── 02/
│   └── 03/
├── outputs/               # Output files from failed test cases
└── results.csv            # Final score summary
```

## File Namings
ZIP file name format:
`NIM-1-PXX_NIM.zip`

Python script inside the ZIP:
`PXX_NIM_YY.py`

XX → Practical assignment number (e.g., 01)

YY → Problem number (e.g., 01, 02, 03)


## Prerequisites
You need to provide your own test and expected output files, as they differ for each grading session.

- `testN.txt`: Input file that will be passed to the student's program (as standard input).
- `expectedN.txt`: File containing the expected output for that test case.


## Output Comparison Logic

The grader first performs a string-based comparison after trimming whitespace.

If strings differ, it extracts numeric values from both outputs and compares them numerically (tolerance 0.01).

If either approach matches, the test is marked as passed.

Runtime errors, timeouts, or missing files are logged and counted as failed tests.