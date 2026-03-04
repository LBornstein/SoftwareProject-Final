#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

//helper functions------------------------------------------------------------------

int *get_matrix_size(char *file_name){
    /*Reads input file in order to determine the matrix size*/
    FILE *file_ptr = fopen(file_name,"r");
    int first_line, character, last_char, *mat_shape = calloc(2,sizeof(int));

    if (file_ptr == NULL || mat_shape == NULL){ 
        /*Check for mem errors*/
        printf("An Error Has Occurred\n");
        if (file_ptr != NULL) fclose(file_ptr);
        if (mat_shape != NULL) free(mat_shape);
        
        exit(1);
    }
    first_line = 1;
    last_char = '\n'; /*helper value to handle last line ending*/

    while((character = fgetc(file_ptr)) != EOF){
         if (first_line == 1){
            if(character == ',') mat_shape[1]++;
            if (character == '\n'){
                mat_shape[0]++;
                mat_shape[1]++;
                first_line = 0;
                last_char = character;
                continue;
            } 
        }
        if (character == '\n' && last_char != '\n') {
            mat_shape[0]++;
        }
        last_char = character;
    }
    if (last_char != '\n' && mat_shape[1] > 0) mat_shape[0]++;

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

//parse points from input file------------------------------------------------------------------

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

//calculation part------------------------------------------------------------------

double** calculate_similarity_matrix(double** p, int n, int d){
    double** result = allocate_matrix(n,n);
    int i, j;
    double dist, power;

    for (i = 0; i < n; i++) {
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
    double** result = allocate_matrix(n,n);
    
    for (int i = 0; i < n; i++) {
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

void calculate_D_pow(double** D, int n){
    for(int i=0; i < n; i++){
        D[i][i] = pow(D[i][i], -0.5);
    }
}

double** calculate_Laplacian(double** D, double** A, int n){
    calculate_D_pow(D, n);
    double** DA = matrix_mult(D, A, n , n, n);
    double** W = matrix_mult(DA, D, n, n, n);
    free_matrix(DA);
    return W;
}





//functions for the C API wrapper------------------------------------------------------------------
double** symnmf(double** H, double** W, int n){
    // need to add this and the function for the H part
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

//Main------------------------------------------------------------------

int main(int argc, char *argv[]) {
    int dim, num_of_points, *mat_shape, **points_mat;
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
        print_matrix(sym(points_mat,num_of_points,dim),num_of_points,num_of_points);
    }

    else if (strcmp(argv[1],"ddg") == 0) {
        print_matrix(ddg(points_mat,num_of_points,dim),num_of_points,num_of_points);
    }

    else if (strcmp(argv[1],"norm") == 0) {
        print_matrix(norm(points_mat,num_of_points,dim),num_of_points,num_of_points);
    }
    else {
        printf("An Error Has Occurred\n");
        return 1;
    }





}

