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

    points_list = parse_points()

    n = len(points_list)
    d = len(points_list[0])

    max_iter = 300
    epsilon = 1e-4

    if goal == 'symnmf':
        W = symnmf.norm(points_list, n ,d)
        H = init_H(W, k)
        M = symnmf.symnmf(H, W, n, k)
    elif goal == 'sym':
        M = symnmf.sym(points_list, n, d)
    elif goal == 'ddg':
        M = symnmf.ddg(points_list, n, d)
    elif goal == 'norm':
        M = symnmf.norm(points_list, n, d)
    else:
        print("An Error Has Occurred")
        sys.exit(1)
    
    output_mat(M)
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
    file = open(file_name, 'r')
    points_arr = []
    s = file.readline()
    while s != "":
        temp = s.strip()
        temp = temp.split(",")
        for i in range(len(temp)):
            temp[i]=float(temp[i])
        points_arr.append(temp)
        s = file.readline()
    return points_arr

def output_mat(m):
    s = ""
    for i in range(len(m)):
        for j in range(len(m[i])):
            s = s + "," + str(m[i][j])
        s = s[:-1] + "\n"
    s = s[:-1]
    print(s)



if (__name__=="__main__"):
    main()