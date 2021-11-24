double covariance(double *values1, double *values2, double mean1, double mean2, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++)
        sum += (values1[i] - mean1) * (values2[i] - mean2);
    return sum / (double)(n - 1);
}