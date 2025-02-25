# Performance Analysis and Results

## Taylor Series Implementation Benchmark

We conducted performance tests on three different implementations: serial, vectorized, and hybrid (combining threads with SIMD) versions. Tests were performed on hardware supporting 12 threads, with the hybrid version utilizing 24 threads.

### Execution Time Comparison
| Implementation | Time (seconds) |
|---------------|---------------|
| Serial        | 1.99769      |
| Vectorized    | 0.974898     |
| Hybrid        | 0.221826     |

### Performance Gains
| Optimization | Speedup Factor |
|-------------|----------------|
| Vectorization | 2.04913x     |
| Hybrid        | 9.00565x     |

The hybrid implementation demonstrates significant performance improvements, achieving a ~9x speedup over the baseline serial version.
