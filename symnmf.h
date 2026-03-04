#ifndef SYMNMF_H
#define SYMNMF_H

/* Memory management prototypes used by the Python API wrapper */
double **allocate_matrix(int rows, int cols);
void free_matrix(double **matrix);

/* Core algorithm prototypes */
double** sym(double** data_points, int n, int d);
double** ddg(double** data_points, int n, int d);
double** norm(double** data_points, int n, int d);
double** symnmf(double** H, double** W, int n, int k, double eps, int max_iter);

#endif /* SYMNMF_H */
