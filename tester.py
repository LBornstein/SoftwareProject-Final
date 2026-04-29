import os
import subprocess
import numpy as np
import re
import sys
import time

# --- CONFIGURATION ---
TEST_FILE = "test_data.txt"
TIMEOUT_SEC = 15  
VALGRIND_CMD = ["valgrind", "--leak-check=full", "--error-exitcode=1", "--quiet"]

# ANSI Colors 
GREEN = '\033[92m'
RED = '\033[91m'
YELLOW = '\033[93m'
CYAN = '\033[96m'
RESET = '\033[0m'

def print_header(msg):
    print(f"\n{YELLOW}{'='*70}\n{msg}\n{'='*70}{RESET}")

def print_success(msg):
    print(f"{GREEN}[PASSED]{RESET} {msg}")

def print_error(msg):
    print(f"{RED}[FAILED]{RESET} {msg}")
    sys.exit(1)

def run_command(cmd, valgrind=False):
    """Runs a shell command and returns stdout and returncode, with timeout."""
    if valgrind:
        cmd = VALGRIND_CMD + cmd
        
    try:
        result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=TIMEOUT_SEC)
        return result.returncode, result.stdout.strip()
    except subprocess.TimeoutExpired:
        return -1, "TIMEOUT EXPIRED (Infinite loop or severe performance degradation detected)"
    except Exception as e:
        return -1, str(e)

# --- MATHEMATICAL GROUND TRUTH ---
def calc_sym(X):
    n = X.shape[0]
    sym = np.zeros((n, n))
    for i in range(n):
        for j in range(n):
            if i != j:
                sym[i, j] = np.exp(-np.sum((X[i] - X[j])**2) / 2.0)
    return sym

def calc_ddg(sym):
    n = sym.shape[0]
    ddg = np.zeros((n, n))
    for i in range(n):
        ddg[i, i] = np.sum(sym[i])
    return ddg

def calc_norm(sym, ddg):
    n = sym.shape[0]
    d_inv_sqrt = np.zeros((n, n))
    for i in range(n):
        if ddg[i, i] > 0:
            d_inv_sqrt[i, i] = 1.0 / np.sqrt(ddg[i, i])
    return np.dot(np.dot(d_inv_sqrt, sym), d_inv_sqrt)

def parse_output_to_matrix(output_str):
    """Converts the formatted string output back into a numpy array for math comparison."""
    try:
        lines = [line.split(',') for line in output_str.split('\n') if line.strip()]
        return np.array(lines, dtype=float)
    except ValueError:
        return None

def validate_matrix_format_and_math(output_str, expected_rows, expected_cols, true_matrix=None):
    """Evaluates output formatting to exactly %.4f and checks mathematical accuracy if provided."""
    if not output_str:
        return False, "Output is empty. Expected matrix."
        
    lines = output_str.split('\n')
    if len(lines) != expected_rows:
        return False, f"Expected {expected_rows} rows, got {len(lines)}."
        
    pattern = re.compile(r"^-?\d+\.\d{4}(,-?\d+\.\d{4})*$")
    
    for i, line in enumerate(lines):
        if not pattern.match(line):
            return False, f"Row {i+1} violates format (%.4f):\n-> {line}"
        if len(line.split(',')) != expected_cols:
             return False, f"Row {i+1} has {len(line.split(','))} columns, expected {expected_cols}."
             
    if true_matrix is not None:
        parsed_matrix = parse_output_to_matrix(output_str)
        if parsed_matrix is None:
             return False, "Failed to parse output matrix for math validation."
        if not np.allclose(parsed_matrix, true_matrix, atol=1e-4):
             return False, "Mathematical assertion failed. Output does not match the expected ground-truth matrix."

    return True, "Valid"

# --- TEST PHASES ---
def generate_test_data(filename, n, d, edge_case=False):
    np.random.seed(42)
    if edge_case:
        # Clustered data to test convergence
        data = np.vstack([np.random.normal(0, 0.1, (n//2, d)), np.random.normal(5, 0.1, (n - n//2, d))])
    else:
        data = np.random.rand(n, d) * 10
        
    # FIX: Explicitly round the data to 4 decimal places before calculating ground truth.
    # This prevents compounding precision errors in norm due to C reading truncated strings.
    data = np.round(data, decimals=4)
    np.savetxt(filename, data, fmt="%.4f", delimiter=",")
    return data

def test_argument_edge_cases():
    print_header("Phase 1: Argument Parsing & Error Handling")
    expected_err = "An Error Has Occurred"
    
    edge_cases = [
        ("Python: Missing arguments", ["python3", "symnmf.py"]),
        ("Python: Invalid Goal", ["python3", "symnmf.py", "3", "fake_goal", TEST_FILE]),
        ("C: Missing arguments", ["./symnmf"]),
        ("C: Invalid Goal", ["./symnmf", "fake_goal", TEST_FILE]),
    ]

    for desc, cmd in edge_cases:
        code, out = run_command(cmd, valgrind=(cmd[0] == "./symnmf"))
        if expected_err not in out:
            print_error(f"Edge Case Failed: {desc}\nExpected: '{expected_err}'\nGot: '{out}'")
        else:
            print_success(f"Handled correctly: {desc}")

def test_core_logic_parity_and_math(X, k, profile_name):
    n = X.shape[0]
    print_header(f"Phase 2 & 3: Math Truth, Parity & Memory - Profile: {profile_name} (N={n})")
    
    # Calculate Ground Truths
    true_sym = calc_sym(X)
    true_ddg = calc_ddg(true_sym)
    true_norm = calc_norm(true_sym, true_ddg)
    
    goals = {
        "sym": true_sym,
        "ddg": true_ddg,
        "norm": true_norm
    }
    
    c_outputs = {}

    # Test C Executable against Ground Truth
    for goal, true_matrix in goals.items():
        code, out = run_command(["./symnmf", goal, TEST_FILE], valgrind=True)
        if code != 0:
            print_error(f"C execution failed or Valgrind found leaks for '{goal}'.")
            
        valid, msg = validate_matrix_format_and_math(out, n, n, true_matrix)
        if not valid:
            print_error(f"C Error in '{goal}': {msg}")
            
        c_outputs[goal] = out
        print_success(f"C passed '{goal}' (Memory Clean, Format Valid, Math Verified)")

    # Test Python Parity against C
    for goal in goals.keys():
        code, out = run_command(["python3", "symnmf.py", str(k), goal, TEST_FILE])
        if code != 0:
            print_error(f"Python execution failed for '{goal}'.")
        if out != c_outputs[goal]:
            print_error(f"Parity mismatch in '{goal}'. Python output != C output.")
        print_success(f"Python C-API passed '{goal}' (Matches C perfectly)")

    # Test Full SymNMF Optimization
    print(f"\n{CYAN}--- Running Full SymNMF Optimization ---{RESET}")
    code, out = run_command(["python3", "symnmf.py", str(k), "symnmf", TEST_FILE])
    if code != 0:
        print_error("Python symnmf failed.")
    valid, msg = validate_matrix_format_and_math(out, n, k)  # Only format checked here
    if not valid:
        print_error(f"Python symnmf format error: {msg}")
    print_success("Full SymNMF optimization formatted correctly and ran without crashing.")

def test_analysis_module(k):
    print_header("Phase 4: Analysis Module Validation")
    code, out = run_command(["python3", "analysis.py", str(k), TEST_FILE])
    if code != 0:
        print_error(f"analysis.py failed with return code {code}.\nOutput:\n{out}")
    
    # Strictly validate analysis output (expects two float scores for K-Means and SymNMF)
    lines = [line.strip() for line in out.split('\n') if line.strip()]
    if len(lines) < 2:
         print_error("analysis.py did not output enough lines. Expected silhouette scores.")
         
    for i, line in enumerate(lines[:2]):
        if not re.search(r"nmf|kmeans", line, re.IGNORECASE):
             print_error(f"analysis.py output line {i+1} missing algorithm identifier (nmf/kmeans): {line}")
        if not re.search(r"\d+\.\d{4}", line):
             print_error(f"analysis.py output line {i+1} missing float formatted to 4 decimals: {line}")
             
    print_success("analysis.py executed successfully and output contains formatted metrics.")

def main():
    print_header("SymNMF Rigorous Testing Initiated")
    
    print("Building project...")
    os.system("rm -f *.so symnmf") 
    code, out = run_command(["make"])
    if code != 0: print_error(f"Make failed:\n{out}")
    
    code, out = run_command(["python3", "setup.py", "build_ext", "--inplace"])
    if code != 0: print_error(f"setup.py failed:\n{out}")
    print_success("Build successful.")

    # --- TEST PROFILES ---
    profiles = [
        {"name": "Standard Data", "N": 30, "D": 5, "K": 3, "edge": False},
        {"name": "Clustered Edge Case", "N": 40, "D": 3, "K": 2, "edge": True},
        {"name": "High Dimensional", "N": 100, "D": 15, "K": 5, "edge": False}
    ]

    test_argument_edge_cases()

    for p in profiles:
        X = generate_test_data(TEST_FILE, p["N"], p["D"], edge_case=p["edge"])
        test_core_logic_parity_and_math(X, p["K"], p["name"])

    test_analysis_module(k=3)

    print_header("ALL RIGOROUS TESTS PASSED SUCCESSFULLY.")
    if os.path.exists(TEST_FILE): os.remove(TEST_FILE)

if __name__ == "__main__":
    main()
