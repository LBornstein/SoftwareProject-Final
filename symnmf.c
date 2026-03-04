#include <stdio.h>
#include <math.h>
#include <stdlib.h>





static double calculate_distance(double p[], double q[], int n){
    /*Euclidian Distance function*/
    double sum = 0.0;
    int i;
    for (i = 0; i < n; i++) {
        sum += (p[i] - q[i]) * (p[i] - q[i]);
    }
    return sqrt(sum);
}


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


