double variance(double *values, double mean, int n) {

    double sum = 0.0;
    for (int i = 0; i < n; i++)
        sum += (values[i] - mean) * (values[i] - mean);
    return sum / (double)(n - 1);

}