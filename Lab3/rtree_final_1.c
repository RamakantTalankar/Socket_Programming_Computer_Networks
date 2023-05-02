#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>


struct BoundDefiner
{
    int dmin;
    int dmax;
};

int max (int x, int y){
    return (x>y)? x: y;
}

int min (int x, int y){
    return (x<y)? x: y;
}

typedef struct BoundDefiner *Bounds;
struct Node
{
    int num_of_children_or_tuples; // if leaf node, then num of tuples, else num of children
    struct BoundDefiner *bounddefiners;
    struct Node * parent;
    struct Node **child_nodes;
    int **list_of_tuples; // only for leaf nodes
};

struct Rtree
{
    int max_entries;
    int min_entries;
    struct Node *root;
    int numofdimensions;
};

struct Node* nodeSplit(struct Rtree *rtree, struct Node *parent_node, struct Node *child_node);
struct Node* adjust_tree(struct Rtree *rtree, struct Node *node1, struct Node * node2);
struct Node *nodeSplit_leaf(struct Rtree* rtree, struct Node *leaf_node, int *tuple);

struct Node *new_node(int numofdimensions)
{
    struct Node *node = malloc(sizeof(struct Node));
    node->num_of_children_or_tuples = 0;
    node->bounddefiners = malloc(sizeof(struct BoundDefiner)*numofdimensions); // changed
    node->child_nodes = NULL;
    node->parent = NULL;
    node->list_of_tuples = NULL;
    return node;
}

struct Rtree *new_rtree(int max_entries, int min_entries, int numofdimensions)
{
    struct Rtree *rtree = malloc(sizeof(struct Rtree));
    rtree->max_entries = max_entries;
    rtree->min_entries = min_entries;
    rtree->numofdimensions = numofdimensions;
    rtree->root = NULL;
    return rtree;
}

int getArea(int numofdimensions, Bounds bounddefiners)
{
    int area = 1;
    for (int i = 0; i < numofdimensions; i++)
    {
        area *= (bounddefiners[i].dmax - bounddefiners[i].dmin);
    }
    return area;
}

Bounds get_bounding_box(int numofdimensions, Bounds bounddefiners, Bounds bounddefiners2)
{
    Bounds bounding_box = malloc(sizeof(struct BoundDefiner) * numofdimensions);
    for (int i = 0; i < numofdimensions; i++)
    {
        bounding_box[i].dmin = min(bounddefiners[i].dmin, bounddefiners2[i].dmin);
        bounding_box[i].dmax = max(bounddefiners[i].dmax, bounddefiners2[i].dmax);
    }
    return bounding_box;
};

int intersects(int numofdimensions, Bounds bounddefiners, Bounds bounddefiners2)
{
    // return area increased from r2 if r1 intersects, else return -1
    double area = 1;
    for (int i = 0; i < numofdimensions; i++)
    {
        if (bounddefiners[i].dmin > bounddefiners2[i].dmax || bounddefiners[i].dmax < bounddefiners2[i].dmin)
        {
            return -1;
        }
        area *= (max(bounddefiners[i].dmax, bounddefiners2[i].dmax) - min(bounddefiners[i].dmin, bounddefiners2[i].dmin)); 
    }
    return area - getArea(numofdimensions, bounddefiners2);
}

bool is_leaf(struct Node *n)
{
    //return n->num_of_children_or_tuples == 0; 
    return (n->list_of_tuples != NULL); //changed
}

struct searchResultStruct
{
    int **list_of_tuples;
    int num_of_tuples;
};

typedef struct searchResultStruct *searchResult;

searchResult createSearchResult(int **list_of_tuples, int num_of_tuples)
{
    searchResult result = malloc(sizeof(struct searchResultStruct));
    result->list_of_tuples = list_of_tuples;
    result->num_of_tuples = num_of_tuples;
    return result;
}

int checkIfTupleInBounds(Bounds bounddefiners, int *tuple, int numofdimensions)
{
    for (int i = 0; i < numofdimensions; i++)
    {
        if (tuple[i] < bounddefiners[i].dmin || tuple[i] > bounddefiners[i].dmax)
        {
            return 0;
        }
    }
    return 1;
}

searchResult searchTuplesInGivenBounds(int numofdimensions, Bounds bounddefiners, struct Node *node)
{
    if (intersects(numofdimensions, bounddefiners, node->bounddefiners) == -1)
    {
        return NULL;
    }
    else
    {
        if (is_leaf(node))
        {
            int **list_of_tuples = NULL;
            int numofresults = 0;
            for (int i = 0; i < node->num_of_children_or_tuples; i++)
            {
                if (checkIfTupleInBounds(bounddefiners, node->list_of_tuples[i], numofdimensions))
                {
                    list_of_tuples = realloc(list_of_tuples, sizeof(int *) * (numofresults + 1));
                    list_of_tuples[numofresults] = node->list_of_tuples[i];
                    numofresults++;
                }
            }
            return createSearchResult(list_of_tuples, numofresults);
        }
        else
        {
            int **list_of_tuples = NULL;
            int numofresults = 0;
            for (int i = 0; i < node->num_of_children_or_tuples; i++)
            {
                searchResult searchresultfromchild = searchTuplesInGivenBounds(numofdimensions, bounddefiners, node->child_nodes[i]);
                if (searchresultfromchild != NULL)
                {
                    list_of_tuples = realloc(list_of_tuples, sizeof(int *) * (numofresults + searchresultfromchild->num_of_tuples));
                    for (int j = 0; j < searchresultfromchild->num_of_tuples; j++)
                    {
                        list_of_tuples[numofresults] = searchresultfromchild->list_of_tuples[j];
                        numofresults++;
                    }
                }
            }
            return createSearchResult(list_of_tuples, numofresults);
        }
    }
}

void printTuple(int *tuple, int numofdimensions)
{
    printf("(");
    for (int i = 0; i < numofdimensions; i++)
    {
        printf("%d", tuple[i]);
        if (i != numofdimensions - 1)
        {
            printf(", ");
        }
    }
    printf(")");
}

void searchRTree(Bounds bounddefiners, struct Rtree *rtree)
{
    if (rtree->root == NULL)
    {
        printf("Tree empty\n");
        return;
    }
    searchResult result = searchTuplesInGivenBounds(rtree->numofdimensions, bounddefiners, rtree->root);
    if (result == NULL)
    {
        printf("No tuples found in given bounds\n");
    }
    else
    {
        printf("%d Tuple(s) found in given bounds :\n", result->num_of_tuples);
        for (int i = 0; i < result->num_of_tuples; i++)
        {
            printTuple(result->list_of_tuples[i], rtree->numofdimensions);
            printf("\n");
        }
    }
}

void printInternalNodeFromBounds(Bounds bounddefiners, int numofdimensions)
{
    // print all min() and max() values
    int *min = malloc(sizeof(int) * numofdimensions);
    int *max = malloc(sizeof(int) * numofdimensions);
    for (int i = 0; i < numofdimensions; i++)
    {
        min[i] = bounddefiners[i].dmin;
        max[i] = bounddefiners[i].dmax;
    }
    printTuple(min, numofdimensions);
    printf(" ");
    printTuple(max, numofdimensions);
    printf("\n");
}

void printNode(struct Node *node, int numofdimensions)
{
    if (is_leaf(node))
    {
        printf("Leaf Node: ");
        for (int i = 0; i < node->num_of_children_or_tuples; i++)
        {
            printTuple(node->list_of_tuples[i], numofdimensions);
            printf(" ");
        }
        printf("\n");
    }
    else
    {
        printf("Internal Node with %d children. Bounds: ", node->num_of_children_or_tuples);
        printInternalNodeFromBounds(node->bounddefiners, numofdimensions);
        for (int i = 0; i < node->num_of_children_or_tuples; i++)
        {
            printNode(node->child_nodes[i], numofdimensions);
        }
    }
}

void printRtree(struct Rtree *rtree)
{
    if (rtree->root == NULL)
    {
        printf("Tree empty\n");
        return;
    }
    printNode(rtree->root, rtree->numofdimensions);
}

int getAreaEnlargedOnInclusion(int numofdimensions, Bounds bounddefiners, int *tuple) //new method for area enlarged on enclusion
{
    int orig_area = getArea(numofdimensions, bounddefiners);
    int new_area = 1;

    for(int i=0; i<numofdimensions; i++)
    {
        if(tuple[i]<bounddefiners[i].dmin)
            new_area *= (bounddefiners[i].dmax - tuple[i]);
        else if(tuple[i] > bounddefiners[i].dmax)
            new_area *= (tuple[i]-bounddefiners[i].dmin);
        else
            new_area *= bounddefiners[i].dmax - bounddefiners[i].dmin;
    } 
    return new_area-orig_area;
}    

struct Node *chooseLeaf(int *tuple, struct Node *node, int numofdimensions)
{
    if (is_leaf(node))
    {
        return node;
    }
    else
    {
        int minarea = INT_MAX;
        int minareaindex = -1;
        int minareaenlargedoninclusion;
        for (int i = 0; i < node->num_of_children_or_tuples; i++)
        {
            int areaenlargedoninclusion = getAreaEnlargedOnInclusion(numofdimensions, node->child_nodes[i]->bounddefiners, tuple);
            if (areaenlargedoninclusion < minareaenlargedoninclusion)
            {
                minareaenlargedoninclusion = areaenlargedoninclusion;
                minareaindex = i;
            }
            else if (areaenlargedoninclusion == minareaenlargedoninclusion)
            {
                // choose the one with minimum area (not area enlarged on inclusion
                int area = getArea(numofdimensions, node->child_nodes[i]->bounddefiners);
                if (area < minarea)
                {
                    minarea = area;
                    minareaindex = i;
                }
            }
            //return chooseLeaf(tuple, node->child_nodes[minareaindex], numofdimensions); //changed to outer loop
        }
        return chooseLeaf(tuple, node->child_nodes[minareaindex], numofdimensions); // to outer loop
    }
}

struct Node *createLeafNode(int numofdimensions, int *tuple)
{
    struct Node *leaf = malloc(sizeof(struct Node));
    leaf->num_of_children_or_tuples = 1;
    leaf->list_of_tuples = malloc(sizeof(int *));
    leaf->list_of_tuples[0] = tuple;
    leaf->bounddefiners = malloc(sizeof(Bounds) * numofdimensions);
    leaf->parent = NULL;
    // leaf->bounddefiners = malloc(sizeof(BoundDefiner) * numofdimensions); //must be this ?
    for (int i = 0; i < numofdimensions; i++)
    {
        leaf->bounddefiners[i].dmin = tuple[i];
        leaf->bounddefiners[i].dmax = tuple[i];
    }
    return leaf;
}

struct Node *addTupleToLeafNode(int numofdimensions, int *tuple, struct Node *node)
{
    node->num_of_children_or_tuples++;
    node->list_of_tuples = realloc(node->list_of_tuples, sizeof(int *) * (node->num_of_children_or_tuples)); 
    node->list_of_tuples[node->num_of_children_or_tuples - 1] = tuple;
    for (int i = 0; i < numofdimensions; i++)
    {
        if (tuple[i] < node->bounddefiners[i].dmin)
        {
            node->bounddefiners[i].dmin = tuple[i];
        }
        if (tuple[i] > node->bounddefiners[i].dmax)
        {
            node->bounddefiners[i].dmax = tuple[i];
        }
    }
    return node;
}


void pulli_ka_dulli(struct Rtree * rtree)
{
    Bounds bb = rtree->root->child_nodes[0]->bounddefiners;
            for (int i=1; i<rtree->root->num_of_children_or_tuples; i++)
            {
                 bb = get_bounding_box(rtree->numofdimensions, bb, rtree->root->child_nodes[i]->bounddefiners);
            }
            rtree->root->bounddefiners = bb;
}

void insert(struct Rtree *rtree, int *tuple)
{
    if (rtree->root == NULL) // If the tree is empty, create a new root
    {
        rtree->root = new_node(rtree->numofdimensions);
        rtree->root->list_of_tuples = malloc(sizeof(int *));
        rtree->root->list_of_tuples[0] = tuple;
        rtree->root->num_of_children_or_tuples = 1;
        rtree->root->child_nodes = NULL;
        rtree->root->parent = NULL;
        for (int i = 0; i < rtree->numofdimensions; i++) // Update the root's bounding box
        {
            rtree->root->bounddefiners[i].dmin = tuple[i];
            rtree->root->bounddefiners[i].dmax = tuple[i];
        }
    }
    else // If the tree is not empty, traverse it to find the appropriate leaf node to insert the tuple
    {
        struct Node *leaf_node = chooseLeaf(tuple, rtree->root, rtree->numofdimensions);
        struct Node *split_leaf_node = NULL;

        // Add the tuple to the leaf node
        if (leaf_node->num_of_children_or_tuples < rtree->max_entries) // If there is space in the leaf node
        {
            leaf_node = addTupleToLeafNode(rtree->numofdimensions, tuple , leaf_node);
        }
        else // If there is no space in the leaf node, split it
        {
            split_leaf_node = nodeSplit_leaf(rtree, leaf_node, tuple);
        }

        struct Node * split_root = adjust_tree(rtree, leaf_node, split_leaf_node);
        if(split_root != NULL) // if root split, create new node for split
        {
            struct Node *new_root =  new_node(rtree->numofdimensions);
            new_root->bounddefiners = get_bounding_box(rtree->numofdimensions, rtree->root->bounddefiners, split_root->bounddefiners);
            new_root->child_nodes = malloc(sizeof(struct Node *) * (2));
            new_root->num_of_children_or_tuples = 2;
            new_root->child_nodes[0] = rtree->root;
            new_root->child_nodes[1] = split_root;
            rtree->root = new_root;
            new_root->child_nodes[0]->parent = new_root;
            new_root->child_nodes[1]->parent = new_root;
        }
        else if(!is_leaf(rtree->root))
        { // Need to adjust MBR of root
            pulli_ka_dulli(rtree);
        }
    }
}


struct Node *adjust_tree(struct Rtree *rtree, struct Node * node1, struct Node * node2){
    //Step 1 : if N == root
    if (node1 == rtree->root)
        return node2;

    //Step 2: Changing MBR of node1 if node not split
    if (node2 == NULL)
    {
        if(!is_leaf(node1))
        {
            Bounds bb1 = node1->child_nodes[0]->bounddefiners;
            for (int i=1; i<node1->num_of_children_or_tuples; i++)
            {
                bb1 = get_bounding_box(rtree->numofdimensions, bb1, node1->child_nodes[i]->bounddefiners);
            }
            node1->bounddefiners = bb1;
        }
        else
        {
            struct Node* dummyNode = createLeafNode(rtree->numofdimensions, node1->list_of_tuples[0]);
            Bounds bb2 = dummyNode->bounddefiners;
            free(dummyNode);
            for(int i=1; i<node1->num_of_children_or_tuples; i++){
                struct Node* dummyNode2 = createLeafNode(rtree->numofdimensions, node1->list_of_tuples[i]);
                bb2 = get_bounding_box(rtree->numofdimensions, dummyNode2->bounddefiners,bb2);
                free(dummyNode2);
            }
            node1->bounddefiners = bb2;
        }

    }

    struct Node * parent_split = NULL;
    //Step 3: Propogate Node Split
    if (node1->parent->num_of_children_or_tuples < rtree->max_entries -1 && node2!=NULL){ // to add node to parent node's list of child nodes
        node1->parent->child_nodes = realloc(node1->parent->child_nodes, (sizeof(struct Node) * (node1->parent->num_of_children_or_tuples +1)));
        node1->parent->child_nodes[node1->parent->num_of_children_or_tuples] = node2;
        node1->parent->num_of_children_or_tuples++;
    }   
    else if(node1->parent->num_of_children_or_tuples == rtree->max_entries && node2 !=NULL){ // to split node
        parent_split = nodeSplit(rtree, node1->parent, node2);
    }

    //Step 4: Move upto next level
    struct Node * dummy = adjust_tree(rtree, node1->parent, parent_split);
    return NULL;
    
}

// NOTES for nodeSplit
// 1. struct Node * nodeSplit(int numofdimensions, struct Node *parent_node, struct Node excess_child_node)
// 2. All nodes after nodeSplit must have MBR adjusted !! (Very Imp)
// 3. Use Quadratic Splitting for PickNext

struct SplitArray {
    struct Node ** arr;
    int n;
};

void removeNodeFromSplitArray(struct SplitArray * temp, int ind){
    for (int j=ind; j<temp->n-1; j++){
        temp->arr[j] = temp->arr[j+1];
    }
    temp->arr[temp->n-1] = NULL;
    temp->n--;
}

struct Node** pickSeed(int numofdimensions, struct SplitArray * temp){
    int d = INT_MIN;
    int ind1, ind2;
    for (int i=0; i<temp->n; i++){
         for (int j=i; j<temp->n; j++){
             int d2 = getArea(numofdimensions,get_bounding_box(numofdimensions, temp->arr[i]->bounddefiners, temp->arr[j]->bounddefiners)) - getArea(numofdimensions, temp->arr[i]->bounddefiners) - getArea(numofdimensions, temp->arr[j]->bounddefiners);
             if (d2>d){
                 d = d2;
                 ind1 = i;
                 ind2 = j;
            }
         }
     }
    struct Node** ans = malloc(sizeof(struct Node*) * 2);

    
    ans[0] = temp->arr[ind1];
    ans[1] = temp->arr[ind2];
    int ind_1 = max(ind1, ind2);
    removeNodeFromSplitArray(temp, ind_1);
    removeNodeFromSplitArray(temp, ind1+ind2-ind_1);
    return ans;
}

void addChildNode2Parent(struct Node* parent_node, struct Node* child_node, int numofdimensions){
    parent_node->num_of_children_or_tuples++;
    if(parent_node->child_nodes == NULL)
        parent_node->child_nodes = malloc(sizeof(struct Node*) * parent_node->num_of_children_or_tuples);
    else
        parent_node->child_nodes = realloc(parent_node->child_nodes, sizeof(struct Node*) * parent_node->num_of_children_or_tuples);
    parent_node->child_nodes[parent_node->num_of_children_or_tuples - 1] = child_node;
    child_node->parent = parent_node;
    for(int i=0; i< numofdimensions; i++){
        parent_node->bounddefiners[i].dmax = max(child_node->bounddefiners[i].dmax, parent_node->bounddefiners[i].dmax);
        parent_node->bounddefiners[i].dmin = min(child_node->bounddefiners[i].dmin, parent_node->bounddefiners[i].dmin);
    } 
}

void pickNext(struct Node* node1, struct Node* node2, struct SplitArray* temp, int numofdimensions){
    int d = INT_MIN;
    struct Node * node_temp;
    int ind1;
    for (int i=0; i<temp->n; i++){
        int d1 = getArea(numofdimensions,get_bounding_box(numofdimensions, temp->arr[i]->bounddefiners, node1->bounddefiners)) - getArea(numofdimensions, node1->bounddefiners);
        int d2 = getArea(numofdimensions,get_bounding_box(numofdimensions, temp->arr[i]->bounddefiners, node2->bounddefiners)) - getArea(numofdimensions, node2->bounddefiners);
        if(abs(d1-d2) > d){
            d = abs(d1-d2);
            node_temp = (d1>d2)? node2: node1;
            ind1 = i;
        }
    }
    addChildNode2Parent(node_temp, temp->arr[ind1], numofdimensions);
    removeNodeFromSplitArray(temp, ind1);
}

struct Node* nodeSplit(struct Rtree *rtree, struct Node *parent_node, struct Node *child_node){
    struct SplitArray* temp = malloc(sizeof(struct SplitArray));
    temp->arr = malloc(sizeof(struct Node *) * (parent_node->num_of_children_or_tuples +1));
    temp->arr[0] = child_node;
    temp->n = parent_node->num_of_children_or_tuples +1;
    struct Node * grandparent_node = parent_node->parent;
    
    //initializing nodes for splitting
    for(int i=0; i<parent_node->num_of_children_or_tuples; i++)
       temp->arr[i+1] = parent_node->child_nodes[i];

    struct Node * split_node = new_node(rtree->numofdimensions);
    free(parent_node->child_nodes);
    parent_node->child_nodes = NULL;
    parent_node->child_nodes = malloc(sizeof(struct Node *));
    for(int i=0; i<rtree->numofdimensions; i++){
         parent_node->bounddefiners->dmax = INT_MIN;
         parent_node->bounddefiners->dmin = INT_MAX;
         split_node->bounddefiners->dmax = INT_MIN;
         split_node->bounddefiners->dmin = INT_MAX;
    }
    parent_node->num_of_children_or_tuples =0;
    split_node->num_of_children_or_tuples =0;
    

    //Step 1: Call pickSeed
    struct Node** pick_seed_node = pickSeed(rtree->numofdimensions, temp);
    addChildNode2Parent(parent_node, pick_seed_node[0], rtree->numofdimensions);
    addChildNode2Parent(split_node, pick_seed_node[1], rtree->numofdimensions);
    for(int i=0; i<rtree->numofdimensions; i++){
        parent_node->bounddefiners[i].dmax = pick_seed_node[0]->bounddefiners[i].dmax;
        parent_node->bounddefiners[i].dmin = pick_seed_node[0]->bounddefiners[i].dmin;
        split_node->bounddefiners[i].dmax = pick_seed_node[1]->bounddefiners[i].dmax;
        split_node->bounddefiners[i].dmin = pick_seed_node[1]->bounddefiners[i].dmin;
    }

    //Step 2: Call pickNext
    for (int i=0; i<rtree->max_entries-1; i++){
        pickNext(parent_node, split_node, temp, rtree->numofdimensions);
    }
    free(temp->arr);
    free(temp);
    parent_node->parent = grandparent_node;

    return split_node;
}

void convertChild2Tuple(struct Node * parent_node, int numofdimensions){
    if(parent_node == NULL)
        return;

    parent_node->list_of_tuples = realloc(parent_node->list_of_tuples, (sizeof(int*) * parent_node->num_of_children_or_tuples));
    for(int i=0; i<parent_node->num_of_children_or_tuples; i++){
        parent_node->list_of_tuples[i] = malloc(sizeof(int)*numofdimensions);
        for (int j=0; j<numofdimensions; j++){
            parent_node->list_of_tuples[i][j] = parent_node->child_nodes[i]->bounddefiners[j].dmax;
        }
    }

}

struct Node *nodeSplit_leaf(struct Rtree* rtree, struct Node *leaf_node, int *tuple){
    //converting tuples to child nodes
    struct Node * tuple_node = createLeafNode(rtree->numofdimensions, tuple);
    for( int j=0; j<rtree->numofdimensions; j++){
        leaf_node->bounddefiners[j].dmax = INT_MIN;
        leaf_node->bounddefiners[j].dmin = INT_MAX;
    }
    int old_value = leaf_node->num_of_children_or_tuples;
    leaf_node->num_of_children_or_tuples = 0;
    for (int i=0; i<old_value; i++){
        addChildNode2Parent(leaf_node, createLeafNode(rtree->numofdimensions,leaf_node->list_of_tuples[i]), rtree->numofdimensions);
    }
    leaf_node->list_of_tuples = NULL;
    
    
    struct Node * split_leaf_node = nodeSplit(rtree, leaf_node, tuple_node);

    //converting child nodes to tuple
    convertChild2Tuple(leaf_node, rtree->numofdimensions);
    convertChild2Tuple(split_leaf_node, rtree->numofdimensions);
    return split_leaf_node;

}




int main (){
    struct Rtree * rtree = malloc(sizeof(struct Rtree));
    rtree->max_entries =4;
    rtree->min_entries =2;
    rtree->numofdimensions =2;

    int * tuple = malloc(sizeof(int)*2);
    tuple[0] = 0;
    tuple[1] = 1;

    int * tuple2 = malloc(sizeof(int)*2);
    tuple2[0] = 2;
    tuple2[1] = 3;

    int * tuple3 = malloc(sizeof(int)*2);
    tuple3[0] = 4;
    tuple3[1] = 5;

    int * tuple4 = malloc(sizeof(int)*2);
    tuple4[0] = 9;
    tuple4[1] = 3;

    int * tuple5 = malloc(sizeof(int)*2);
    tuple5[0] = 7;
    tuple5[1] = 8;

    int * tuple6 = malloc(sizeof(int)*2);
    tuple6[0] =10;
    tuple6[1] = 15;

    insert(rtree, tuple);
    insert(rtree, tuple2);
    insert(rtree, tuple3);
    insert(rtree, tuple4);
    insert(rtree, tuple5);
    printRtree(rtree);
    insert(rtree, tuple6);
    printRtree(rtree);
    return 0;
}