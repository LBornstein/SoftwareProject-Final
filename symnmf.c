#include <stdio.h>
#include <math.h>






static double calculate_distance(double p[], double q[], int n) {
    double sum = 0.0;
    int i;
    for (i = 0; i < n; i++) {
        sum += (p[i] - q[i]) * (p[i] - q[i]);
    }
    return sqrt(sum);
}
