import numpy as np
import sys
import math

def main():
    np.random.seed(1234)

    if len(sys.argv) != 4:
        print("An Error Has Occurred")
        sys.exit(1)
    
    k = int(sys.argv[1])
    goal = sys.argv[2]
    file_name = sys.argv[3]

    points_list = parse_points()

    max_iter = 300
    epsilon = 1e-4

    if goal == 'symnmf'():
        handle_symnmf
    elif goal == 'sym':
        handle_sym
    elif goal == 'ddg':
        handle_ddg
    elif goal == 'norm':
        handle_norm
    else:
        print("An Error Has Occurred")
        sys.exit(1)
    
    exit(0)


def parse_points(file_name):
    pass


def handle_symnmf():
    pass
def handle_sym():
    pass
def handle_ddg():
    pass
def handle_norm():
    pass

def output_mat(m):
    pass


if (__name__=="__main__"):
    main()