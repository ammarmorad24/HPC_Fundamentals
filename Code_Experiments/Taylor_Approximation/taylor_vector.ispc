// taylor_vector.ispc
export void sinx_ispc(uniform int N, uniform int terms, 
                      uniform double* uniform x, 
                      uniform double* uniform y) {
    // Process elements in SIMD-friendly chunks
    foreach (i = 0 ... N) {
        varying double value = x[i];
        varying double x_squared = value * value;
        varying double numerator = x_squared * value;
        varying double denominator = 6.0;
        varying int sign = -1;
        
        for (uniform int j = 1; j <= terms; j++) {
            value += sign * numerator / denominator;
            numerator *= x_squared;
            denominator *= (2.0 * j + 2.0) * (2.0 * j + 3.0);
            sign *= -1;
        }
        
        y[i] = value;
    }
}






