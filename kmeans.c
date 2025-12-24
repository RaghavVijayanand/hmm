#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DIM 39          // MFCC dimension
#define K 100           // number of clusters
#define MAX_POINTS 10000
#define MAX_ITER 100
#define EPS 1e-4

// ---------- Data structures ----------

typedef struct {
    double x[DIM];
} Vector;

typedef struct {
    Vector centroid;
    int count;
} Cluster;

// ---------- Distance function ----------

double squared_distance(Vector a, Vector b)
{
    double sum = 0.0;
    for (int i = 0; i < DIM; i++)
    {
        double d = a.x[i] - b.x[i];
        sum += d * d;
    }
    return sum;
}

// ---------- Initialize clusters ----------

void initialize_clusters(Vector *data, Cluster *clusters)
{
    for (int k = 0; k < K; k++)
    {
        clusters[k].centroid = data[k]; // pick first K points
        clusters[k].count = 0;
    }
}

// ---------- One K-means iteration ----------

double kmeans_iteration(Vector *data, int N, Cluster *clusters)
{
    Vector sum[K];

    // Reset accumulators
    for (int k = 0; k < K; k++)
    {
        clusters[k].count = 0;
        for (int d = 0; d < DIM; d++)
            sum[k].x[d] = 0.0;
    }

    // Assignment step
    for (int n = 0; n < N; n++)
    {
        int best = 0;
        double best_dist = squared_distance(data[n], clusters[0].centroid);

        for (int k = 1; k < K; k++)
        {
            double dist = squared_distance(data[n], clusters[k].centroid);
            if (dist < best_dist)
            {
                best_dist = dist;
                best = k;
            }
        }

        clusters[best].count++;
        for (int d = 0; d < DIM; d++)
            sum[best].x[d] += data[n].x[d];
    }

    // Update step
    double error = 0.0;
    for (int k = 0; k < K; k++)
    {
        if (clusters[k].count == 0)
            continue;

        for (int d = 0; d < DIM; d++)
        {
            double old = clusters[k].centroid.x[d];
            clusters[k].centroid.x[d] = sum[k].x[d] / clusters[k].count;
            error += fabs(old - clusters[k].centroid.x[d]);
        }
    }

    return error;
}

