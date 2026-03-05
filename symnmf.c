#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "symnmf.h"

#define BETA 0.5
/*Helper Function Prototype Declarations --------------------------------------------------*/
double **allocate_matrix(int rows, int cols);
void free_matrix(double **matrix);
void print_matrix(double **matrix, int rows, int cols);
double **mat_transpose(double **mat, int rows, int cols);
double calculate_distance(double* p, double* q, int d);
double **matrix_mult(double **LeftMat, double **RightMat, int rows_left, int inner_dim, int cols_right);
double **matrix_subtract(double **LeftMat, double **RightMat, int rows, int cols);
double calculate_Frobenius_norm(double **matrix, int rows, int cols);

/*helper functions------------------------------------------------------------------*/

int *get_matrix_size(char *file_name){
    /* Reads input file in order to determine the matrix size */
    FILE *file_ptr = fopen(file_name, "r");
    int character, last_char = '\n'; 
    int *mat_shape = calloc(2, sizeof(int));

    if (file_ptr == NULL || mat_shape == NULL) { 
        /* Check for mem errors */
        printf("An Error Has Occurred\n");
        if (file_ptr != NULL) fclose(file_ptr);
        if (mat_shape != NULL) free(mat_shape);
        exit(1);
    }

    while ((character = fgetc(file_ptr)) != EOF) {
        /* Count columns only in the first row by counting commas */
        if (mat_shape[0] == 0 && character == ',') {
            mat_shape[1]++;
        }
        /* Count row only if we hit a newline and the prev char was not a newline */
        if (character == '\n' && last_char != '\n') {
            mat_shape[0]++;
        }
        last_char = character;
    }

    /* Edge case: File contains data but does not end with a newline */
    if (last_char != '\n' && last_char != EOF) {
        mat_shape[0]++;
    }

    /* The number of columns is exactly the number of commas + 1 */
    if (mat_shape[0] > 0) {
        mat_shape[1]++;
    }

    fclose(file_ptr);
    return mat_shape;
}

double **allocate_matrix(int rows, int cols) {
    /* Allocates a continuous 2D matrix */
    int i;
    double **matrix_pointers;
    double *matrix_data;

    matrix_pointers = calloc(rows, sizeof(double *));
    matrix_data = calloc(rows * cols, sizeof(double));

    /* Check for memory errors */
    if (matrix_pointers == NULL || matrix_data == NULL) {
        printf("An Error Has Occurred\n");
        if (matrix_pointers != NULL) free(matrix_pointers);
        if (matrix_data != NULL) free(matrix_data);
        exit(1);
    }

    /* Map the pointers to the block */
    for (i = 0; i < rows; i++) {
        matrix_pointers[i] = matrix_data + (i * cols);
    }

    return matrix_pointers;
}


void free_matrix(double **matrix) {
    /* Safely frees the points matrix */
    if (matrix != NULL) {
        free(matrix[0]); /* Free the data block first */
        free(matrix);    /* Free the array of row pointers */
    }
}

void print_matrix(double **matrix, int rows, int cols) {
    int i, j;
    
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            /* Format to 4 decimal places */
            printf("%.4f", matrix[i][j]);
            
            /* Print comma separator, but not after the last element */
            if (j < cols - 1) {
                printf(",");
            }
        }
        /* New line for each row */
        printf("\n");
    }
}

double **mat_transpose(double **mat, int rows, int cols){
    double **ret_mat = allocate_matrix(cols, rows);
    int i, j;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            ret_mat[j][i] = mat[i][j];
        }
    }
    return ret_mat;
}


double **update_H(double **H, double **W, int n, int k){
    /*function to update H using the provided formula*/
    double **ret_mat, **WH, **transH, **HTH, **HHTH;
    int i, j;
    ret_mat = allocate_matrix(n,k);
    WH = matrix_mult(W,H,n,n,k);
    transH = mat_transpose(H,n,k);
    HTH = matrix_mult(transH,H,k,n,k); 
    HHTH = matrix_mult(H,HTH,n,k,k);

    free_matrix(transH);
    free_matrix(HTH);

    for(i = 0; i < n ; i++){
        for (j = 0 ; j < k ; j++){
            ret_mat[i][j] = H[i][j]*(1-BETA + BETA*(WH[i][j]/HHTH[i][j]));
        }
    }

    free_matrix(WH);
    free_matrix(HHTH);

    return ret_mat;
}


/*parse points from input file------------------------------------------------------------------*/

double **parse_points(char *file_name, int num_of_points, int dim){
    /* Reads the points from the provided .txt file */
    int i,j;
    double **points_arr;
    FILE *file_ptr;
    points_arr = allocate_matrix(num_of_points, dim);
    
    file_ptr = fopen(file_name, "r");

    if (file_ptr == NULL) {
        printf("An Error Has Occurred\n");
        free_matrix(points_arr);
        exit(1);
    }

    for (i = 0; i < num_of_points; i++) {
        for (j = 0; j < dim; j++) {
            /* %lf reads the double, %*c reads and drops the comma or newline */
            if (fscanf(file_ptr, "%lf%*c", &points_arr[i][j]) != 1) {
                printf("An Error Has Occurred\n");
                free_matrix(points_arr);
                fclose(file_ptr);
                exit(1);
            }
        }
    }
    fclose(file_ptr);
    return points_arr;

}

/*calculation part------------------------------------------------------------------*/
double calculate_distance(double* p, double* q, int d) {
    double sum = 0.0;
    int i;
    for (i = 0; i < d; i++) {
        sum += (p[i] - q[i]) * (p[i] - q[i]);
    }
    return sqrt(sum); /*kept this as sqrt even though we square it, in order to be more modular (more future functions might want to use Euclidian Distance)*/
}

double** calculate_similarity_matrix(double** p, int n, int d){
    double** result = allocate_matrix(n,n);
    int i, j;
    double dist, power;

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++){
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
    int i,j;
    double row_sum,**result = allocate_matrix(n,n);
    
    for (i = 0; i < n; i++) {
        row_sum = 0.0;
        for (j = 0; j < n; j++) {
            row_sum += p[i][j];
        }

        result[i][i] = row_sum;
    }
    
    return result;
}



double **matrix_mult(double **LeftMat, double **RightMat, int rows_left, int inner_dim, int cols_right){
    /*Matrix multipication function, we assume dimensions are correct and dont check them*/
    int i,j,k; 
    double **ret_mat = allocate_matrix(rows_left,cols_right);

    for (i = 0; i < rows_left ; i++){
        for (j = 0 ; j < cols_right ; j++){
            for(k = 0 ; k < inner_dim ; k++){
                ret_mat[i][j] += LeftMat[i][k]*RightMat[k][j]; 
            }
        }
    }

    return ret_mat;
}

double **matrix_subtract(double **LeftMat, double **RightMat, int rows, int cols){
    /*Matrix subtraction function*/
    int i, j;
    double **ret_mat = allocate_matrix(rows, cols);

    for (i = 0; i < rows; i++){
        for (j = 0; j < cols; j++){
            ret_mat[i][j] = LeftMat[i][j] - RightMat[i][j];
        }
    }

    return ret_mat;
}

void calculate_D_pow(double** D, int n){
    int i;
    for( i = 0; i < n; i++){
        D[i][i] = pow(D[i][i], -0.5);
    }
}

double** calculate_Laplacian(double** D, double** A, int n){
    double **DA, **W;
    calculate_D_pow(D, n);
    DA = matrix_mult(D, A, n , n, n);
    W = matrix_mult(DA, D, n, n, n);
    free_matrix(DA);
    return W;
}

double calculate_Frobenius_norm(double **matrix, int rows, int cols){
    /*calculates the F_norm, returns the raw squared values instead of the sqrt since we square it up again later*/
    int i,j;
    double retval = 0;

    for(i = 0; i < rows ; i++){
        for(j = 0; j < cols ; j++){
            retval += matrix[i][j] * matrix[i][j];
        }
    }

    return retval;
}



/*functions for the C API wrapper------------------------------------------------------------------*/
double** symnmf(double** H, double** W, int n , int k, double eps, int max_iter){
    double **new_H, **temp;
    int i,convergence;
    i = 0;
    convergence = 0;

    for(i = 0 ; i < max_iter ; i++){
        new_H = update_H(H,W,n,k);
        temp = matrix_subtract(new_H,H,n,k);
        if(calculate_Frobenius_norm(temp,n,k) < eps) convergence = 1;
        free_matrix(temp);
        free_matrix(H);
        H = new_H;
        if(convergence) break;
    }
    
    return H;
}

double** sym(double** data_points, int n, int d){
    return calculate_similarity_matrix(data_points, n, d);
}

double** ddg(double** data_points, int n, int d){
    double **A = calculate_similarity_matrix(data_points, n, d);
    double **ret = calculate_diagonal_degree_matrix(A, n);
    free_matrix(A);
    return ret;
}

double** norm(double** data_points, int n, int d){
    double **A = calculate_similarity_matrix(data_points, n, d);
    double **D = calculate_diagonal_degree_matrix(A, n);
    double **ret = calculate_Laplacian(D, A, n);
    free_matrix(A);
    free_matrix(D);
    return ret;
}

/*Main------------------------------------------------------------------*/

int main(int argc, char *argv[]) {
    int dim, num_of_points, *mat_shape;
    double **points_mat, **output_mat;
    if (argc != 3){
        printf("An Error Has Occurred\n");
        return 1;
    }

    /* get matrix shape for further calculations */
    mat_shape = get_matrix_size(argv[2]);
    dim = mat_shape[1];
    num_of_points = mat_shape[0];
    free(mat_shape);
    
    points_mat = parse_points(argv[2],num_of_points,dim);

    if (strcmp(argv[1],"sym") == 0) {
        output_mat = sym(points_mat,num_of_points,dim);
    }

    else if (strcmp(argv[1],"ddg") == 0) {
       output_mat = ddg(points_mat,num_of_points,dim);
    }

    else if (strcmp(argv[1],"norm") == 0) {
        output_mat = norm(points_mat,num_of_points,dim);
    }
    else {
        printf("An Error Has Occurred\n");
        return 1;
    }

    print_matrix(output_mat,num_of_points,num_of_points);
    free_matrix(output_mat);
    free_matrix(points_mat);
    return 0;
}

