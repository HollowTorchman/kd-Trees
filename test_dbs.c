#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "dbscan.h"
#include "kd_trees.h"

int main() {
    int num_points = 10;
    int k = 2;
    double points[] = {
        0.0, 0.0,  
        0.1, 0.1,  
        10.0, 10.0,
        0.0, 0.2,  
        5.0, 5.0,
        8.0, 8.0,  
        8.1, 8.1,  
        20.0, 15.0,
        0.0, 0.2,  
        4.0, 4.0      
    };
    double epsilon = 1.0;
    int min_pts = 2;
    int *clusters = dbscan_cluster(points, num_points, k, epsilon, min_pts);

    for (int i = 0; i < num_points; i++) {
        printf("Point %d: [%.1f %.1f] Cluster %d\n", i, points[i*k], points[i*k+1], clusters[i]);
    }

    assert(clusters[0] == clusters[1]);
    assert(clusters[1] == clusters[3]);

    free(clusters);
    return 0;
}