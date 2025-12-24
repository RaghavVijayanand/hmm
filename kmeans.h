#ifndef KMEANS_H
#define KMEANS_H

#define DIM 39
#define K 100

typedef struct {
    double x[DIM];
} Vector;

typedef struct {
    Vector centroid;
    int count;
} Cluster;

// Distance
double squared_distance(Vector a, Vector b);

// Initialization
void initialize_clusters(Vector *data, Cluster *clusters);

// One iteration
double kmeans_iteration(Vector *data, int N, Cluster *clusters);

#endif
