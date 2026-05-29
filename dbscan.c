#include "dbscan.h"
#include <stdio.h>
#include <stdlib.h>

/*
Queue for storing nodes during core point expansion
*/
typedef struct {
    int *data;
    int capacity;
    int size;
} NeighborQueue;

static void queue_init(NeighborQueue *q, int initial_capacity) {
    q->data = (int *)malloc(initial_capacity * sizeof(int));
    q->capacity = initial_capacity;
    q->size = 0;
}

static void queue_push(NeighborQueue *q, int val) {
    if (q->size >= q->capacity) {
        q->capacity *= 2;
        q->data = (int *)realloc(q->data, q->capacity * sizeof(int));
    }
    q->data[q->size++] = val;
}

static void queue_free(NeighborQueue *q) {
    free(q->data);
}

/*
Count entries in result list
*/
static int count_results(KdResultNode *results) {
    int count = 0;
    while (results != NULL) {
        count++;
        results = results->next;
    }
    return count;
}

int* dbscan_cluster(const double *points, int num_points, int k, double epsilon, int min_pts) {
    if (points == NULL || num_points <= 0 || k <= 0 || epsilon < 0.0 || min_pts < 1) {
        return NULL;
    }

    // construct kd-tree from point array
    KdTree *tree = kd_create(k);
    for (int i = 0; i < num_points; i++) {
        kd_insert(tree, &points[i * k], i);
    }

    // init tracking variables
    DBScanPoint *point_states = (DBScanPoint *)malloc(num_points * sizeof(DBScanPoint));
    int *assignments = (int *)malloc(num_points * sizeof(int));
    for (int i = 0; i < num_points; i++) {
        point_states[i].cluster_id = UNKNOWN;
        point_states[i].visited = false;
    }
    int current_cluster_id = 0;

    // main algorithm
    for (int i = 0; i < num_points; i++) {
        // skip visited points
        if (point_states[i].visited) {
            continue;
        }

        // mark new point as visited
        point_states[i].visited = true;
        
        // count neighbors in epsilon range around point
        KdResultNode *neighbors = kd_range_search(tree, &points[i * k], epsilon);
        int neighbor_count = count_results(neighbors);

        // check if point is a core point
        if (neighbor_count < min_pts) {
            point_states[i].cluster_id = NONCORE;
            kd_free_results(neighbors);
        } else {
            // assign cluster id
            point_states[i].cluster_id = current_cluster_id;

            // init queue for storing neighbors during core point expansion
            NeighborQueue expansion_queue;
            queue_init(&expansion_queue, neighbor_count * 2);

            // push neighbor points to queue
            KdResultNode *curr = neighbors;
            while (curr != NULL) {
                int point_idx = curr->node->id;
                queue_push(&expansion_queue, point_idx);
                curr = curr->next;
            }
            kd_free_results(neighbors);

            // cluster growth
            for (int j = 0; j < expansion_queue.size; j++) {
                int neighbor_idx = expansion_queue.data[j];

                if (!point_states[neighbor_idx].visited) {
                    point_states[neighbor_idx].visited = true;

                    KdResultNode *sub_neighbors = kd_range_search(tree, &points[neighbor_idx * k], epsilon);
                    int sub_count = count_results(sub_neighbors);

                    // if neighbor is a core point expand it
                    if (sub_count >= min_pts) {
                        KdResultNode *sub_curr = sub_neighbors;
                        while (sub_curr != NULL) {
                            int sub_idx = sub_curr->node->id;
                            queue_push(&expansion_queue, sub_idx);
                            sub_curr = sub_curr->next;
                        }
                    }
                    kd_free_results(sub_neighbors);
                }

                // after core point expansion claim all non-core points in cluster radius
                if (point_states[neighbor_idx].cluster_id == UNKNOWN || 
                    point_states[neighbor_idx].cluster_id == NONCORE) {
                    point_states[neighbor_idx].cluster_id = current_cluster_id;
                }
            }

            queue_free(&expansion_queue);
            current_cluster_id++; 
        }
    }

    // output
    for (int i = 0; i < num_points; i++) {
        assignments[i] = point_states[i].cluster_id;
    }

    printf("Found %d clusters\n", current_cluster_id);

    free(point_states);
    kd_free(tree);

    return assignments;
}

void dbscan_export_to_csv(const char* filename, const double* points, 
                          const int* cluster_labels, int num_points, int k) {
    FILE* foutput = fopen(filename, "w");

    fprintf(foutput, "id,");
    for (int i = 0; i < k; i++) {
        fprintf(foutput, "dim%d,", i);
    }
    fprintf(foutput, "cluster_id\n");

    for (int i = 0; i < num_points; i++) {
        fprintf(foutput, "%d,", i); // ID

        for (int dim = 0; dim < k; dim++) {
            fprintf(foutput, "%f,", points[i * k + dim]);
        }
        
        fprintf(foutput, "%d\n", cluster_labels[i]);
    }

    fclose(foutput);
    printf("Exported to %s\n", filename);
}