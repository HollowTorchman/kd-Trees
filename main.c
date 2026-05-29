#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kd_trees.h"
#include "dbscan.h"

int get_k_from_csv(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return -1;

    char line[4096];
    if (!fgets(line, sizeof(line), file)) {
        fclose(file);
        return -1;
    }
    fclose(file);

    int k = 0;
    char *token = strtok(line, ",");
    while (token != NULL) {
        k++;
        token = strtok(NULL, ",");
    }
    return k;
}

double* load_array_from_csv(const char *filename, int k, int *out_num_points) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return NULL;
    }

    int capacity = 1000;
    int count = 0;
    double *points = (double *)malloc(capacity * k * sizeof(double));

    char line[4096];
    while (fgets(line, sizeof(line), file)) {

        if (line[0] == '\n' || line[0] == '#' || line[0] == '\0') continue;

        if (count >= capacity) {
            capacity *= 2;
            points = (double *)realloc(points, capacity * k * sizeof(double));
        }

        int dim = 0;
        char *token = strtok(line, ",");
        while (token != NULL && dim < k) {
            points[count * k + dim] = strtod(token, NULL);
            dim++;
            token = strtok(NULL, ",");
        }

        if (dim == k) {
            count++;
        }
    }

    fclose(file);
    *out_num_points = count;
    printf("Loaded %d points (%d dimensions) from %s\n", count, k, filename);
    return points;
}


int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: ./robot_spatial <file.csv> <operation> [parameters] (-p)\n");
        printf("Use -p flag at the end to printing the tree to console");
        printf("Operations:\n");
        printf(" -kd_nearest <coords>     Find nearest neighbor (e.g., 1.0,2.0)\n");
        printf(" -dbscan <eps,minPts>     Run DBSCAN clustering (e.g., 0.5,5)\n");
        return 1;
    }

    const char *filename = argv[1];
    const char *operation = argv[2];

    int k = get_k_from_csv(filename);
    if (k <= 0) {
        fprintf(stderr, "Error: Could not determine dimensions from %s\n", filename);
        return 1;
    }

    int num_points = 0;
    double *points = load_array_from_csv(filename, k, &num_points);
    if (!points || num_points == 0) {
        fprintf(stderr, "Error: Failed to load dataset.\n");
        if (points) free(points);
        return 1;
    }

    if (strcmp(operation, "-kd_nearest") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Error: Invalid format for -kd_nearest\n");
            free(points);
            return 1;
        }

        double *target = (double *)malloc(k * sizeof(double));
        int dim = 0;
        char *token = strtok(argv[3], ",");
        while (token != NULL && dim < k) {
            target[dim++] = strtod(token, NULL);
            token = strtok(NULL, ",");
        }

        if (dim != k) {
            fprintf(stderr, "Error: Invalid format for -kd_nearest.\n", k);
            free(target);
            free(points);
            return 1;
        }

        KdTree *tree = kd_create(k);
        for (int i = 0; i < num_points; i++) {
            kd_insert(tree, &points[i * k], i);
        }

        KdNode *nearest = kd_nearest(tree, target);
        if (nearest) {
            printf("\nNearest Neighbor Found: [");
            for (int i = 0; i < k; i++) {
                printf("%.2f%s", nearest->point[i], (i < k - 1) ? ", " : "");
            }
            printf("]");
            printf(" (id = %d)\n", nearest->id + 1);
        } else {
            printf("\nTree search error.\n");
        }

        kd_free(tree);
        free(target);

    } else if (strcmp(operation, "-dbscan") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Error: Invalid format for -dbscan\n");
            free(points);
            return 1;
        }

        double epsilon = 0.0;
        int min_pts = 0;
        if (sscanf(argv[3], "%lf,%d", &epsilon, &min_pts) != 2) {
            fprintf(stderr, "Error: Invalid format for -dbscan\n");
            free(points);
            return 1;
        }

        printf("Running DBSCAN (Epsilon: %.2f, MinPts: %d)\n", epsilon, min_pts);
        
        int *labels = dbscan_cluster(points, num_points, k, epsilon, min_pts);
        if (labels) {
            const char *out_file = "dbscan_output.csv";
            dbscan_export_to_csv(out_file, points, labels, num_points, k);
            free(labels);
        } else {
            fprintf(stderr, "DBSCAN error.\n");
        }

    } else {
        fprintf(stderr, "Unknown operation '%s'\n", operation);
    }

    if (argv[4]) {
        if (strcmp(argv[4], "-p") == 0) {
            KdTree *tree = kd_create(k);
            for (int i = 0; i < num_points; i++) {
                kd_insert(tree, &points[i * k], i);
            }
            printf("\n--- K-D Tree Structure ---\n");
            kd_print(tree);
            kd_free(tree);
        }
    }

    free(points);
    return 0;
}