#include <stdio.h>
#include <math.h>


double** caluclate_similarity_matrix(double** p, int n, int d){
    double** result = (double**)malloc(n * sizeof(double*));
    int i, j;
    double dist, power;

    for (i = 0; i < n; i++) {
        result[i] = (double*)calloc(n, sizeof(double));
        for (j=0; j < n; j++){
            if (i != j){
                dist = calculate_distance(p[i], p[j], d);
                power = -(dist * dist) / 2.0;
                result[i][j] = exp(power);
            }
        }
    }

    return result;
}


double** calculate_diagonal_degree_matrix(double** p, int n) {
    double** result = (double**)malloc(n * sizeof(double*));
    
    for (int i = 0; i < n; i++) {
        result[i] = (double*)calloc(n, sizeof(double));

        double row_sum = 0.0;
        for (int j = 0; j < n; j++) {
            row_sum += p[i][j];
        }
        
        result[i][i] = row_sum;
    }
    
    return result;
}

double calculate_distance(double* p, double* q, int d) {
    double sum = 0.0;
    int i;
    for (i = 0; i < d; i++) {
        sum += (p[i] - q[i]) * (p[i] - q[i]);
    }
    return sqrt(sum);
}
