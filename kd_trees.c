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
KdNode* kd_create_node(int k, const double *point, int id) {
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

    node->id = id;
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
KdNode* kd_insert_helper(KdNode *current_node, int k, const double* point, int id, int depth) {
    // Create new node if empty spot reached
    if (current_node == NULL) {
        return kd_create_node(k, point, id);
    }

    // Splitting axis for depth
    int axis = depth % k;

    // Compare coordinates by axis, go left if <=, right otherwise
    if (point[axis] < current_node->point[axis]){
        current_node->left = kd_insert_helper(current_node->left, k, point, id, depth + 1); 
    } 
    else {
        current_node->right = kd_insert_helper(current_node->right, k, point, id, depth + 1);
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
void kd_insert(KdTree *tree, const double *point, int id) {
    tree->root = kd_insert_helper(tree->root, tree->k, point, id, 0);
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

#define ANSI_COLOR_RED     "\x1b[31m"   // Axis 0 (X)
#define ANSI_COLOR_GREEN   "\x1b[32m"   // Axis 1 (Y)
#define ANSI_COLOR_YELLOW  "\x1b[33m"   // Axis 2 (Z)
#define ANSI_COLOR_CYAN    "\x1b[36m"   // Axis 3
#define ANSI_COLOR_MAGENTA "\x1b[35m"   // High-dimensional fallback
#define ANSI_COLOR_GRAY    "\x1b[90m"   // Dim color for NULL connections
#define ANSI_COLOR_RESET   "\x1b[0m"    // Resets color back to default terminal color

/*
Recursively print tree as a directory structure color-coded by axis
*/
static void kd_print_helper(KdNode *node, int k, int depth, char *prefix, int is_left) {
    if (node == NULL) {
        return;
    }

    int axis = depth % k;
    char axis_char = (k <= 3) ? ("XYZ"[axis]) : ('0' + axis);

    const char *axis_color;
    switch (axis) {
        case 0:  axis_color = ANSI_COLOR_RED;    break;
        case 1:  axis_color = ANSI_COLOR_GREEN;  break;
        case 2:  axis_color = ANSI_COLOR_YELLOW; break;
        case 3:  axis_color = ANSI_COLOR_CYAN;   break;
        case 4: axis_color = ANSI_COLOR_GRAY; break;
        default: axis_color = ANSI_COLOR_MAGENTA; break;
    }

    printf("%s", prefix);

    printf("%s", axis_color);
    printf("%s|--(Axis %c) [", is_left ? "L" : "R", axis_char);
    
    for (int i = 0; i < k; i++) {
        printf("%.2f", node->point[i]);
        if (i < k - 1) printf(", ");
    }

    printf("]%s\n", ANSI_COLOR_RESET);

    char new_prefix[256];
    snprintf(new_prefix, sizeof(new_prefix), "%s|   ", prefix);

    if (node->left || node->right) {
        if (node->left) {
            kd_print_helper(node->left, k, depth + 1, new_prefix, 1);
        } else {
            printf("%s|%s   L|-- [NULL]%s\n", prefix, ANSI_COLOR_GRAY, ANSI_COLOR_RESET);
        }
        
        if (node->right) {
            kd_print_helper(node->right, k, depth + 1, new_prefix, 0);
        } else {
            printf("%s|%s   R|-- [NULL]%s\n", prefix, ANSI_COLOR_GRAY, ANSI_COLOR_RESET);
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

/*
Compare 2 points
*/
static int points_equal(int k, const double *p1, const double *p2) {
    for (int i = 0; i < k; i++) {
        if (p1[i] != p2[i]) return 0;
    }
    return 1;
}

/*
Find node with minimum value along axis
*/
static KdNode* kd_find_min(KdNode *current_node, int target_axis, int depth, int k) {
    if (current_node == NULL) {
        return NULL;
    }

    int current_axis = depth % k;
    if (current_axis == target_axis) {
        if (current_node->left == NULL) {
            return current_node;
        }
        return kd_find_min(current_node->left, target_axis, depth + 1, k);
    }

    KdNode *left_min = kd_find_min(current_node->left, target_axis, depth + 1, k);
    KdNode *right_min = kd_find_min(current_node->right, target_axis, depth + 1, k);

    KdNode *res = current_node;
    if (left_min != NULL && left_min->point[target_axis] < res->point[target_axis]) {
        res = left_min;
    }
    if (right_min != NULL && right_min->point[target_axis] < res->point[target_axis]) {
        res = right_min;
    }
    return res;
}

/*
Finds and deletes the node recursively
*/
static KdNode* kd_delete_helper(KdNode *current_node, int k, const double *point, int depth) {
    if (current_node == NULL) {
        return NULL;
    }

    int axis = depth % k;

    if (points_equal(k, current_node->point, point)) {
        // if ight subtree exists
        if (current_node->right != NULL) {
            KdNode *min_node = kd_find_min(current_node->right, axis, depth + 1, k);
            memcpy(current_node->point, min_node->point, k * sizeof(double));
            current_node->id = min_node->id;
            current_node->right = kd_delete_helper(current_node->right, k, min_node->point, depth + 1);
        } 
        // if only left subtree exists
        else if (current_node->left != NULL) {
            KdNode *min_node = kd_find_min(current_node->left, axis, depth + 1, k);
            memcpy(current_node->point, min_node->point, k * sizeof(double));
            current_node->id = min_node->id;
            current_node->right = kd_delete_helper(current_node->left, k, min_node->point, depth + 1);
            current_node->left = NULL; 
        } 
        // if leaf node
        else {
            free(current_node->point);
            free(current_node);
            return NULL;
        }
        return current_node;
    }

    if (point[axis] < current_node->point[axis]) {
        current_node->left = kd_delete_helper(current_node->left, k, point, depth + 1);
    } else {
        current_node->right = kd_delete_helper(current_node->right, k, point, depth + 1);
    }

    return current_node;
}

/*
Delete point from tree (Public)
parameters:
 tree
 point - coordinate array of the point to remove
*/
void kd_delete(KdTree *tree, const double *point) {
    tree->root = kd_delete_helper(tree->root, tree->k, point, 0);
}


/*
Add node to result linked list
*/
static void add_result(KdResultNode **head, KdNode *node) {
    KdResultNode *new_res = (KdResultNode*)malloc(sizeof(KdResultNode));
    new_res->node = node;
    new_res->next = *head;
    *head = new_res;
}

/*
Frees result list
*/
void kd_free_results(KdResultNode *results) {
    while (results != NULL) {
        KdResultNode *temp = results;
        results = results->next;
        free(temp);
    }
}

/*
Recursive range search
parameters:
 current_node
 target - target point coordinates
 epsilon_sq - squared search radius
*/
static void kd_range_search_helper(KdNode *current_node, const double *target, double epsilon_sq, 
                                    int depth, int k, KdResultNode **results) {
    if (current_node == NULL) {
        return;
    }

    // calculate dist^2 from target to current
    double dist_sq = 0.0;
    for (int i = 0; i < k; i++) {
        double diff = target[i] - current_node->point[i];
        dist_sq += diff * diff;
    }

    // add to result list if in radius
    if (dist_sq <= epsilon_sq) {
        add_result(results, current_node);
    }

    // navigate tree
    int axis = depth % k;
    double plane_dist = target[axis] - current_node->point[axis];

    KdNode *prim_subtree = (plane_dist < 0) ? current_node->left : current_node->right;
    KdNode *opp_subtree = (plane_dist < 0) ? current_node->right : current_node->left;

    // explore primary subtree
    kd_range_search_helper(prim_subtree, target, epsilon_sq, depth + 1, k, results);

    double plane_dist_sq = plane_dist * plane_dist;
    // explore opposite subtree if it's plane intersects epsilon radius
    if (plane_dist_sq <= epsilon_sq) {
        kd_range_search_helper(opp_subtree, target, epsilon_sq, depth + 1, k, results);
    }
}

/*
Range Search (Public)
parameters:
 tree    - tree wrapper pointer
 target  - center point of search radius coordinates
 epsilon - search radius
returns:
 results - pointer to head of result linked list
*/
KdResultNode* kd_range_search(KdTree *tree, const double *target, double epsilon) {
    if (tree == NULL || tree->root == NULL || target == NULL || epsilon < 0.0) {
        return NULL;
    }
    
    KdResultNode *results = NULL;
    double eps_sq = epsilon * epsilon;
    
    kd_range_search_helper(tree->root, target, eps_sq, 0, tree->k, &results);
    return results;
}