double mean(double *values, int n) {

    double sum = 0.0;
    for (int i = 0; i < n; i++)
        sum += values[i];
    return sum / (double)n;
}