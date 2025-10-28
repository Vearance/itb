import os
import zipfile
import csv
import glob
import subprocess
import shutil
import re

base_dir = "submissions"
test_dir = "tests"
extract_dir = "temp"
output_dir = "outputs"
csv_path = "nilai.csv"

os.makedirs(extract_dir, exist_ok=True)
os.makedirs(output_dir, exist_ok=True)

def normalize_stdout(s):
    return s.strip().replace("\r\n", "\n").replace("\r", "\n")

def filter_relevant_output(s):
    lines = s.splitlines()
    filtered = []
    for line in lines:
        # Skip typical input prompts
        if line.strip().endswith(":"):
            continue
        if line.lower().startswith("masukkan"):
            continue
        if line.lower().startswith("input"):
            continue
        filtered.append(line)
    return "\n".join(filtered).strip()

def numbers_only(s):
    return " ".join(re.findall(r"-?\d+\.?\d*", s))

def safe_write(path, content):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)

def approx_equal(out_str, exp_str, tol=1e-2):
    nums_out = [float(x) for x in re.findall(r"-?\d+\.?\d*", out_str)]
    nums_exp = [float(x) for x in re.findall(r"-?\d+\.?\d*", exp_str)]
    if len(nums_out) != len(nums_exp):
        return False
    return all(abs(a - b) <= tol for a, b in zip(nums_out, nums_exp))

print(f"Working directory: {os.getcwd()}")
print(f"Mencari zip di folder: {base_dir}/")

zip_files = sorted(glob.glob(f"{base_dir}/*.zip"))
if not zip_files:
    print("Tidak ditemukan file zip apapun di folder submissions/.")
    exit()

results = []

for zip_path in zip_files:
    base = os.path.basename(zip_path)
    match = re.match(r"(\d+)-\d+-P(\d+)_([0-9]+)", base)
    if not match:
        print(f"Skip {base} (format nama tidak valid)")
        continue

    nim, prac_num, nim2 = match.groups()
    prac_full = f"P{prac_num}"
    print(f"\nMemproses {base} -> NIM {nim}, Praktikum {prac_full}")

    # Extract zip
    if os.path.exists(extract_dir):
        shutil.rmtree(extract_dir)
    os.makedirs(extract_dir, exist_ok=True)
    with zipfile.ZipFile(zip_path, "r") as z:
        z.extractall(extract_dir)

    row_result = {"NIM": nim, "Praktikum": prac_full}

    for yy in range(1, 4):  # TODO: ganti kalo jumlah file berbeda
        soal_dir = f"{test_dir}/{yy:02d}"
        test_paths = sorted(glob.glob(f"{soal_dir}/test*.txt"))
        print(f"- Menguji soal {yy:02d} ({len(test_paths)} testcases)")

        target_filename = f"P{prac_num}_{nim}_{yy:02d}.py"
        found_scripts = sorted(glob.glob(f"{extract_dir}/**/{target_filename}", recursive=True))

        if not test_paths:
            print(f"  - Tidak ada test case untuk soal {yy:02d}")
            row_result[f"soal{yy:02d}"] = "[no tests]"
            continue

        if not found_scripts:
            print(f"  - File {target_filename} tidak ditemukan")
            row_result[f"soal{yy:02d}"] = "[missing]"
            continue

        script_path = found_scripts[0]
        total_passed = 0
        total_tests = len(test_paths)
        all_outputs = []

        for test_path in test_paths:
            test_id = os.path.basename(test_path).replace("test", "").replace(".txt", "")
            expected_path = f"{soal_dir}/expected{test_id}.txt"

            input_data = open(test_path, "r", encoding="utf-8").read()
            expected_raw = open(expected_path, "r", encoding="utf-8").read().strip() if os.path.exists(expected_path) else ""

            try:
                proc = subprocess.run(
                    ["python", script_path],
                    input=input_data,
                    capture_output=True,
                    text=True,
                    timeout=5
                )
                raw_out = proc.stdout

                # Normalize and filter
                filtered_out = normalize_stdout(filter_relevant_output(raw_out))
                norm_exp = normalize_stdout(expected_raw)

                passed = False
                if (
                    filtered_out == norm_exp
                    or approx_equal(filtered_out, norm_exp)
                    or numbers_only(filtered_out) == numbers_only(norm_exp)
                ):
                    passed = True

                if passed:
                    total_passed += 1
                    print(f"  - Test {test_id}: OK")
                else:
                    print(f"  - Test {test_id}: Mismatch")

                all_outputs.append((test_id, raw_out))

            except subprocess.TimeoutExpired:
                print(f"  - Test {test_id}: Timeout")
                all_outputs.append((test_id, "[timeout]"))
            except Exception as e:
                print(f"  - Test {test_id}: Error ({e})")
                all_outputs.append((test_id, f"[error] {e}"))

        score = round(0.6 * (total_passed / total_tests), 2)
        row_result[f"soal{yy:02d}"] = score
        print(f"  - Skor soal {yy:02d}: {score}")

        if score < 0.6:
            for test_id, content in all_outputs:
                safe_write(f"{output_dir}/{nim}_soal{yy:02d}_test{test_id}.txt", content)

    results.append(row_result)

# Write CSV
fieldnames = sorted(set().union(*[r.keys() for r in results]))

with open(csv_path, "w", newline="", encoding="utf-8") as f:
    writer = csv.DictWriter(f, fieldnames=fieldnames, extrasaction="ignore")
    writer.writeheader()
    for r in results:
        for fn in fieldnames:
            r.setdefault(fn, "")
        writer.writerow(r)

print("\nSelesai. Hasil disimpan di", csv_path)
