#ifndef KD_TREES_H
#define KD_TREES_H

#include <stdlib.h>
#include <string.h>

// Main node
typedef struct KdNode {
    double *point;            // coordinates array
    struct KdNode *left;      // left subtree pointer
    struct KdNode *right;     // right subtree pointer
} KdNode;

// Tree wrapper
typedef struct {
    KdNode *root;             // root pointer
    int k;                    // dimensions
} KdTree;

// Public functions
KdTree* kd_create(int k);
KdNode* kd_create_node(int k, const double *point);
void kd_free(KdTree *tree);
void kd_insert(KdTree *tree, const double *point);
KdNode* kd_nearest(KdTree *tree, const double *target);
void kd_print(KdTree *tree);
void kd_delete(KdTree *tree, const double *point);

#endif //KD_TREES_H