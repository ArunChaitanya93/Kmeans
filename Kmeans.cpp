#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <sys/time.h>
#include <time.h>

#include "sdv_tracing.h"

#define MAX_ITER 100
#define NUM_POINTS 100
#define NUM_CLUSTERS 50

// Structure to represent a 2D point
typedef struct {
    float x;
    float y;
    int cluster;
} Point;


// Function to initialize centroids randomly from the dataset
void initializeCentroids(Point *points, Point *centroids) {
    for (int i = 0; i < NUM_CLUSTERS; i++) {
        int idx = rand() % NUM_POINTS;
        centroids[i].x = points[idx].x;
        centroids[i].y = points[idx].y;
    }
}

// Function to assign points to the nearest centroid
void assignClusters(Point *points, Point *centroids) {
    trace_event_and_value(1000, 1);
    float dx, dy;
    for (int i = 0; i < NUM_POINTS; i++) {
        float minDist = FLT_MAX;
        float distance[NUM_CLUSTERS];
        for (int j = 0; j < NUM_CLUSTERS; j++) {
            dx = points[i].x - centroids[j].x;
            dy = points[i].y - centroids[j].y;
            distance[j] = dx * dx + dy * dy;
            if (distance[j] < minDist) {
                minDist = distance[j];
            }
        }
        int cluster = -1;
        for (int j = 0; j < NUM_CLUSTERS; j++)
        {
            if ((distance[j]-minDist)<1e-3) {
                cluster = j;
                break;
            }
        }
        points[i].cluster = cluster;
    }
    trace_event_and_value(1000, 0);
}

// Function to update centroid positions based on assigned points
void updateCentroids(Point *points, Point *centroids) {
    trace_event_and_value(1000, 3);
    for (int c = 0; c < NUM_CLUSTERS; c++) {
        float sumX = 0.0;
        float sumY = 0.0;
        int count = 0;

        for (int i = 0; i < NUM_POINTS; i++) {
            if (points[i].cluster == c) {
                sumX += points[i].x;
                sumY += points[i].y;
                count++;
            }
        }

        if (count > 0) {
            centroids[c].x = sumX / count;
            centroids[c].y = sumY / count;
        }
    }
    trace_event_and_value(1000, 0);
}

// Main K-means algorithm
void kmeans(Point *points) {
    Point *centroids = (Point *)malloc(NUM_CLUSTERS*sizeof(Point));
    Point *oldCentroids = (Point *)malloc(NUM_CLUSTERS*sizeof(Point));
    initializeCentroids(points, centroids);

    for (int iter = 0; iter < MAX_ITER; iter++) {
        assignClusters(points, centroids);

        trace_event_and_value(1000, 2);
        for (int i = 0; i < NUM_CLUSTERS; i++) {
            oldCentroids[i].x = centroids[i].x;
            oldCentroids[i].y = centroids[i].y;
        }
        trace_event_and_value(1000, 0);

        updateCentroids(points, centroids);

        // Check for convergence
        float maxDist = 0.;
        float dist;
        trace_event_and_value(1000, 4);
        for (int i = 0; i < NUM_CLUSTERS; i++) {
            float dx = (oldCentroids[i].x-centroids[i].x);
            float dy = (oldCentroids[i].y-centroids[i].y);
            dist = dx * dx + dy * dy;
            if(dist > maxDist)
                maxDist = dist;
        }
        trace_event_and_value(1000, 0);

        if (maxDist < 1e-4) {
            printf("Converged after %d iterations.\n", iter + 1);
            free(oldCentroids);
            break;
        }

        if (iter == MAX_ITER - 1) {
            printf("Reached maximum iterations.\n");
            free(oldCentroids);
        }        
    }

    FILE *outputFile = fopen("clustering_results.txt", "w");
    if (outputFile == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Output clustering results
    for (int i = 0; i < NUM_POINTS; i++) {
    fprintf(outputFile, "%.2f\t%.2f\t%d\n",
            points[i].x, points[i].y, points[i].cluster);
    }

    fclose(outputFile);

    free(centroids);
}

int main() {
    // Example dataset
    struct timeval tv1, tv2;
    struct timezone tz;
    double kernel_time;

    const char *v_names[] = {"Other", "Cluster_assignment", "Centroids_swapping", "Centroids_update", "Convergence_check"};
    int values[] = {0, 1, 2, 3, 4};
    trace_name_event_and_values(1000, "code_region", sizeof(values) / sizeof(values[0]), values, v_names);
    trace_init();

    Point *points = (Point *)malloc(NUM_POINTS*sizeof(Point));

    for (int i = 0; i < NUM_POINTS; i++) {
        points[i].x = ((float)rand() / (float)RAND_MAX) * 10.0;  // Random value between 0 and 10
        points[i].y = ((float)rand() / (float)RAND_MAX) * 10.0;  // Random value between 0 and 10        
    }

    gettimeofday(&tv1, &tz);
    kmeans(points);
    gettimeofday(&tv2, &tz);

    kernel_time = (double) (tv2.tv_sec-tv1.tv_sec) + (double) (tv2.tv_usec-tv1.tv_usec) * 1.e-6;

    printf("%-30s: %.3lf\n", "Kernel compute time (seconds)", kernel_time);

    return 0;
}
