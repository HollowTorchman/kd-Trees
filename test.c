#include "kd_trees.h"
#include <stdio.h>

int main() {
    int dimensions = 4;
    KdTree *tree = kd_create(dimensions);

    double points[][4] = {
        {10.0, 20.0, 30.0, 40.0},
        {5.0,  35.0, 15.0, 25.0}, 
        {20.0, 15.0, 50.0, 60.0},
        {2.0,  10.0, 80.0, 90.0}, 
        {4.0,  50.0,  5.0,  8.0},
        {25.0, 5.0,  12.0, 75.0}, 
        {59.0, 44.0, 82.5, 31.0}, 
        {192.0, 168.0, 0.0, 1.0}  
    };
    int num_points = sizeof(points) / sizeof(points[0]);

    for (int i = 0; i < num_points; i++) {
        printf("Inserted: [%.1f, %.1f, %.1f, %.1f]\n", 
               points[i][0], points[i][1], points[i][2], points[i][3]);
        kd_insert(tree, points[i]);
    }
    printf("\n-Default tree-\n");
    kd_print(tree);
    printf("\n");


    double leaf_to_delete[4] = {192.0, 168.0, 0.0, 1.0};
    printf("Deleting node: [%.1f, %.1f, %.1f, %.1f]\n", 
           leaf_to_delete[0], leaf_to_delete[1], leaf_to_delete[2], leaf_to_delete[3]);
    printf("\n");
    kd_delete(tree, leaf_to_delete);
    kd_print(tree);
    printf("\n");


    double node_left_only[4] = {5.0,  35.0, 15.0, 25.0};
    printf("Deleting node: [%.1f, %.1f, %.1f, %.1f]\n", 
           node_left_only[0], node_left_only[1], node_left_only[2], node_left_only[3]);
    printf("\n");
    kd_delete(tree, node_left_only);
    kd_print(tree);
    printf("\n");


    double root_to_delete[4] = {10.0, 20.0, 30.0, 40.0};
    printf("Deleting node: [%.1f, %.1f, %.1f, %.1f]\n", 
           root_to_delete[0], root_to_delete[1], root_to_delete[2], root_to_delete[3]);
    printf("\n");
    kd_delete(tree, root_to_delete);
    kd_print(tree);
    printf("\n");

    printf("memory freed");
    kd_free(tree);

    return 0;
}