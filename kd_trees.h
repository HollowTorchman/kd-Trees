#ifndef KD_TREES_H
#define KD_TREES_H

#include <stdlib.h>
#include <string.h>

// Main node
typedef struct KdNode {
    double *point;            // coordinates array
    int id;                   // dataset position index (for dbscan)
    struct KdNode *left;      // left subtree pointer
    struct KdNode *right;     // right subtree pointer
} KdNode;

// Tree wrapper
typedef struct {
    KdNode *root;             // root pointer
    int k;                    // dimensions
} KdTree;

// Linked node list for storing range search results
typedef struct KdResultNode {
    KdNode *node;                 
    struct KdResultNode *next;
} KdResultNode;

// Public functions
KdTree* kd_create(int k);
KdNode* kd_create_node(int k, const double *point, int id);
void kd_free(KdTree *tree);
void kd_insert(KdTree *tree, const double *point, int id);
KdNode* kd_nearest(KdTree *tree, const double *target);
    void kd_print(KdTree *tree);
void kd_delete(KdTree *tree, const double *point);
KdResultNode* kd_range_search(KdTree *tree, const double *target, double epsilon);
void kd_free_results(KdResultNode *results);


#endif 