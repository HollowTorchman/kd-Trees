#ifndef DBSCAN_H
#define DBSCAN_H

#include "kd_trees.h"
#include <stdbool.h>

#define UNKNOWN -1
#define NONCORE   -2

// Single DBSCAN point structure
typedef struct {
    int cluster_id;  
    bool visited;     
} DBScanPoint;

/*
Computes DBSCAN clustering on point array
parameters:
 points      - (num_points * k) size double array
 num_points  - number of points in dataset
 k           - number of dimensions in dataset
 epsilon     - search radius
 min_pts     - minimum neighbour point density to be considered a core point
returns:
 integer array size (num_points) containing each point's cluster index
*/
int* dbscan_cluster(const double *points, int num_points, int k, double epsilon, int min_pts);

#endif