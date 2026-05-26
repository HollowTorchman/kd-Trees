#include "kd_trees.h"
#include <stdio.h>
#include <math.h>
#include <float.h>

/*
Create an empty k-d tree wrapper. (Public)
parameters:
 k - number of dimensions
return:
 tree - pointer to new KdTree
 NULL on error
*/
KdTree* kd_create(int k) {
    if (k <= 0) {
        fprintf(stderr, "Invalid dimension size");
        return NULL;
    }

    KdTree *tree = (KdTree *)malloc(sizeof(KdTree));
    if (!tree) {
        fprintf(stderr, "Failed to allocate memory");
        return NULL;
    }

    tree->root = NULL;
    tree->k = k;

    return tree;
}

/*
Create a node from a coordinate array (Public)
parameters:
 k - dimension count
 point - coordinate array representing a k-dimensional point
return:
 node - pointer to created node
 NULL on error
*/
KdNode* kd_create_node(int k, const double *point) {
    KdNode *node = (KdNode*)malloc(sizeof(KdNode));
    if (!node) {
        fprintf(stderr, "Failed to allocate memory");
        return NULL;
    }

    node->point = (double*)malloc(k * sizeof(double));
    if (!node->point) {
        fprintf(stderr, "Failed to allocate memory");
        return NULL;
    }
    memcpy(node->point, point, k * sizeof(double));

    node->left = NULL;
    node->right = NULL;

    return node;
}

/*
Frees given node and every node below it recursively 
*/
void kd_free_nodes(KdNode *node) {
    if (!node) return;
    
    kd_free_nodes(node->left);
    kd_free_nodes(node->right);

    free(node->point);
    free(node);
}

/*
Fully frees the tree structure (Public)
parameters:
 tree for removal
*/
void kd_free(KdTree *tree) {
    kd_free_nodes(tree->root);
    free(tree);
}

/*
Recursive function for inserting a point starting from specified depth
recieves:
 current_node - 
 k - dimensionality
 point - coordinate array representing a k-dimensional point
 depth - length of path to root from current node
outputs:
 new created node
*/
KdNode* kd_insert_helper(KdNode *current_node, int k, const double* point, int depth) {
    // Create new node if empty spot reached
    if (current_node == NULL) {
        return kd_create_node(k, point);
    }

    // Splitting axis for depth
    int axis = depth % k;

    // Compare coordinates by axis, go left if <=, right otherwise
    if (point[axis] < current_node->point[axis]){
        current_node->left = kd_insert_helper(current_node->left, k, point, depth + 1); 
    } 
    else {
        current_node->right = kd_insert_helper(current_node->right, k, point, depth + 1);
    }

    // Return next node for recursion
    return current_node;
}

/*
Inserts new point into tree (Public)
parameters:
 tree
 point - coordinate array 
*/
void kd_insert(KdTree *tree, const double *point) {
    tree->root = kd_insert_helper(tree->root, tree->k, point, 0);
}

/*
Computes squared distance between 2 points
d^2 = Sum(i=1..k)(x_i - y_i)^2
*/
double get_squared_distance(int k, const double *point1, const double *point2) {
    double dist = 0.0;
    for (int i = 0; i < k; i++) {
        double diff = point1[i] - point2[i];
        dist += diff * diff;
    }
    return dist;
}

/*
Recursively find nearest neighbor
*/
static void kd_nearest_helper(KdNode *current_node, int k, const double *target, int depth, KdNode **best_node, double *best_dist_sq) {
    if (current_node == NULL) {
        return;
    }

    // Get distance to target point
    double current_dist_sq = get_squared_distance(k, current_node->point, target);

    // Update best distance if current distance is better
    if (current_dist_sq < *best_dist_sq) {
        *best_dist_sq = current_dist_sq;
        *best_node = current_node;
    }

    // Calculate which of the branches is closer to target
    KdNode *close_node;
    KdNode *far_node;
    int axis = depth % k;
    if (target[axis] < current_node->point[axis]) {
        close_node = current_node->left;
        far_node = current_node->right;
    } else {
        close_node = current_node->right;
        far_node = current_node->left;
    }

    // Go down closer branch first
    kd_nearest_helper(close_node, k, target, depth + 1, best_node, best_dist_sq);

    // When backtracking check for possibility of closer point in unexplored branch
    //  calculate squared distance to its axis
    double axis_dist = target[axis] - current_node->point[axis];
    axis_dist *= axis_dist;

    // if distance is less than our best there could be a closer point in the branch
    if (axis_dist < *best_dist_sq) {
        kd_nearest_helper(far_node, k, target, depth + 1, best_node, best_dist_sq);
    }
}

/*
Find nearest neighbour to point (Public)
parameters:
 tree - pointer to KdTree
 target - coordinate array to target point
return:
 pointer to closest KdNode found
 NULL if tree is empty
*/
KdNode* kd_nearest(KdTree *tree, const double *target) {
    if (!tree || !tree->root || !target) {
        return NULL;
    }

    KdNode *best_node = NULL;
    double best_dist_sq = DBL_MAX;
    
    kd_nearest_helper(tree->root, tree->k, target, 0, &best_node, &best_dist_sq);

    return best_node;
}

/*
Recursively print tree as a directory structure
*/
static void kd_print_helper(KdNode *node, int k, int depth, char *prefix, int is_left) {
    if (node == NULL) {
        return;
    }


    int axis = depth % k;
    char axis_char = (k <= 3) ? ("XYZ"[axis]) : ('0' + axis);

    printf("%s", prefix);
    printf("%s|--(Axis %c) [", is_left ? "L" : "R", axis_char);
    
    for (int i = 0; i < k; i++) {
        printf("%.2f", node->point[i]);
        if (i < k - 1) printf(", ");
    }
    printf("]\n");

    char new_prefix[256];
    snprintf(new_prefix, sizeof(new_prefix), "%s    ", prefix);

    if (node->left || node->right) {
        if (node->left) {
            kd_print_helper(node->left, k, depth + 1, new_prefix, 1);
        } else {
            printf("%s    L|-- [NULL]\n", prefix);
        }
        
        if (node->right) {
            kd_print_helper(node->right, k, depth + 1, new_prefix, 0);
        } else {
            printf("%s    R|-- [NULL]\n", prefix);
        }
    }
}

void kd_print(KdTree *tree) {
    if (!tree || !tree->root) {
        printf("Tree is empty\n");
        return;
    }
    printf("Kd-Tree (k = %d):\n", tree->k);
    kd_print_helper(tree->root, tree->k, 0, "", 0);
}