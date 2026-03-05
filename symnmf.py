import numpy as np
import sys
import symnmf

def main():
    np.random.seed(1234)

    if len(sys.argv) != 4:
        print("An Error Has Occurred")
        sys.exit(1)
    
    k = int(sys.argv[1])
    goal = sys.argv[2]
    file_name = sys.argv[3]
    points_list = parse_points(file_name)

    n = len(points_list)
    d = len(points_list[0])

    max_iter = 300
    epsilon = 1e-4

    if goal == 'symnmf':
        W = symnmf.norm(points_list, n ,d)
        H = init_H(W, k)
        M = symnmf.symnmf(H, W, n, k, epsilon, max_iter)

    elif goal == 'sym':
        M = symnmf.sym(points_list, n, d)

    elif goal == 'ddg':
        M = symnmf.ddg(points_list, n, d)

    elif goal == 'norm':
        M = symnmf.norm(points_list, n, d)
        
    else:
        print("An Error Has Occurred")
        sys.exit(1)
    
    print_mat(M)
    exit(0)

def calculate_matrix_avg(M):
    sum = 0
    for i in range(len(M)):
        for j in range(len(M[0])):
            sum += M[i][j]
    return sum / (len(M)**2)
    
def init_H(W, k):
    n = len(W)
    m = calculate_matrix_avg(W)
    interval = 2 * ((m / k) ** 0.5)
    H = [[np.random.uniform(0.0, interval) for col in range(k)] for row in range(n)]
    return H

def parse_points(file_name):
    try:
        points_arr = []
        # Using 'with handles closing the file, even if an error occurs
        with open(file_name, 'r') as file:
            for line in file:
                line = line.strip()
                
                # If the line is empty, skip to the next iteration
                if not line:
                    continue
                
                # Split the line and convert directly to floats using a list comprehension
                temp = line.split(",")
                points_arr.append([float(val) for val in temp])
                
        return points_arr

    # Catching an Exception if one were to occur
    except Exception:
        print("An Error Has Occurred")
        sys.exit(1)

def print_mat(m):
    for row in m:
        # Formats every float to 4 decimal places, joins them with commas
        print(",".join([f"{val:.4f}" for val in row]))



if (__name__=="__main__"):
    main()