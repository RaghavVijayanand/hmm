// ---------- Main ----------

int main()
{
    Vector data[MAX_POINTS];
    Cluster clusters[K];

    int N = 0;

    // Example: read MFCC vectors from file
    FILE *fp = fopen("combined.mfcc", "r");
    if (!fp)
    {
        printf("Error opening file\n");
        return 1;
    }

    while (N < MAX_POINTS)
    {
        for (int d = 0; d < DIM; d++)
        {
            if (fscanf(fp, "%lf", &data[N].x[d]) != 1)
            {
                fclose(fp);
                goto done;
            }
        }
        N++;
    }

done:
    fclose(fp);

    // Initialize clusters
    initialize_clusters(data, clusters);

    // Run K-means
    for (int iter = 0; iter < MAX_ITER; iter++)
    {
        double err = kmeans_iteration(data, N, clusters);
        printf("Iteration %d, error = %f\n", iter + 1, err);

        if (err < EPS)
            break;
    }

    printf("K-means finished.\n");
    return 0;
}
