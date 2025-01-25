#include <stdio.h>  
#include <stdlib.h>
#include <time.h>
int main(){
    int *radius;
    int *diameter;
    double *circumference;
    double *area;
    int size = 1e8;
    radius = (int *)malloc(size * sizeof(int));
    diameter = (int *)malloc(size * sizeof(int));
    circumference = (double *)malloc(size * sizeof(double));
    area = (double *)malloc(size * sizeof(double));
    for (int i = 0; i < size; i++) {
        radius[i] = i % 100;
    }
    // measure time
    clock_t start, end;
    start = clock();
    for (int i = 0; i < size; i++) {
        diameter[i] = 2 * radius[i];
        circumference[i] = 2 * 3.14159 * radius[i];
        area[i] = 3.14159 * radius[i] * radius[i];
    }
    end = clock();
    double time_taken = ((double)end - start) / CLOCKS_PER_SEC;
    printf("Time taken: %f\n", time_taken);
    free(radius);
    free(diameter);
    free(circumference);
    free(area);
    return 0;
}