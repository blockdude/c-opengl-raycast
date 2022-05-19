/*
 * this is only for k-dimensions
 * and has no self balancing
 * properties so performace
 * may degrade during use.
 */

#ifndef K_DIMENTIONAL_TREE
#define K_DIMENTIONAL_TREE

#define KDT_DATA_TYPE float

typedef struct kdtree kdtree;

kdtree *new_kdtree          ( int k, void ( *free_item )( void * ) );
void free_kdtree            ( kdtree *tree );

// getters
int kdt_size                ( kdtree *tree );
int kdt_dim                 ( kdtree *tree );

// build tools
void *kdt_replace           ( kdtree *tree, KDT_DATA_TYPE point[], void *item ); // can return item on insertion or returns existing item on replacement
void *kdt_insert            ( kdtree *tree, KDT_DATA_TYPE point[], void *item ); // can return item on success or returns existing item on failure
void *kdt_remove            ( kdtree *tree, KDT_DATA_TYPE point[] ); // returns node item if point is found otherwise returns NULL
int kdt_delete              ( kdtree *tree, KDT_DATA_TYPE point[] ); // returns 1 on success and 0 on failure

// query tools
void kdt_query_range_func   ( kdtree *tree, KDT_DATA_TYPE point[], KDT_DATA_TYPE range, void ( *func )( void * ) );
void **kdt_query_range      ( kdtree *tree, KDT_DATA_TYPE point[], KDT_DATA_TYPE range, int *length );

void kdt_query_dim_func     ( kdtree *tree, KDT_DATA_TYPE point[], KDT_DATA_TYPE dim[], void ( *func )( void * ) );
void **kdt_query_dim        ( kdtree *tree, KDT_DATA_TYPE point[], KDT_DATA_TYPE dim[], int *length );

// search tools
void *kdt_search            ( kdtree *tree, KDT_DATA_TYPE point[] );
void *kdt_nearest           ( kdtree *tree, KDT_DATA_TYPE point[] );

#endif
