void dct(size_t n, byte_t f[n][n], double F[n][n])
{
	for (size_t u = 0; u < n; ++u)
		for (size_t v = 0; v < n; ++v) {
			double cu = sqrt((1. + !!u) / n),
				   cv = sqrt((1. + !!v) / n);
			double sum = 0;
			for (size_t i = 0; i < n; ++i)
				for (size_t j = 0; j < n; ++j) {
					sum += f[i][j] * cos((2 * i + 1) * M_PI / (2 * n) * u)
						   * cos((2 * j + 1) * M_PI / (2 * n) * v);
				}
			F[u][v] = cu * cv * sum;
		}
}

void idct(size_t n, double F[n][n], byte_t f[n][n])
{
	for (size_t i = 0; i < n; ++i)
		for (size_t j = 0; j < n; ++j) {
			double sum = 0;
			for (size_t u = 0; u < n; ++u)
				for (size_t v = 0; v < n; ++v) {
					double cu = sqrt((1. + !!u) / n),
						   cv = sqrt((1. + !!v) / n);
					sum += cu * cv * F[u][v] * cos((2 * i + 1) * M_PI / (2 * n) * u)
						   * cos((2 * j + 1) * M_PI / (2 * n) * v);
				}
			f[i][j] = round(sum);
		}
}