#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "kd-tree.h"
#include "vc-world.h"

/*
 * typedefs
 */

typedef struct vc_list vc_list;

/*
 * structs
 */

struct vc_object
{
    float x;
    float y;

    vc_world *world;
    vc_object *next;
    vc_object *prev;
    vc_object *above;
    vc_object *below;
    
    int *comp;
};

struct vc_list
{
    vc_object *root;
    int length;
};

struct vc_world
{
    kd_tree *obj_tree;
    vc_list *obj_list;
    int obj_c;
};

struct vc_result
{
    vc_object **objects;
    vc_object *current;
    int length;
    int i;
};

/*
 * util functions
 */

static vc_list *new_vc_list()
{
    vc_list *list = ( vc_list * ) malloc( sizeof( vc_list ) );
    list->root = NULL;
    list->length = 0;
    return list;
}

/*
 * world
 */

vc_world *vc_load_world( char *infile )
{
    vc_world *world = ( vc_world * ) malloc( sizeof( vc_world ) );
    world->obj_tree = new_kd_tree( 2, NULL );
    world->obj_list = new_vc_list();
    world->obj_c = 0;

    if ( infile != NULL )
    {
        FILE *wfile = fopen( infile, "r" );

        if ( wfile != NULL )
        {
            /*
             * load world
             */

            int obj_count;
            fscanf( wfile, "%d", &obj_count );

            for ( int i = 0; i < obj_count; i++ )
            {
                // allocate object
                vc_object *obj = ( vc_object * ) malloc( sizeof( vc_object ) );
                obj->comp = ( int * ) malloc( sizeof( int ) * VC_COMP_LAST );
                obj->world = NULL;
                obj->next = NULL;
                obj->prev = NULL;
                obj->above = NULL;
                obj->below = NULL;

                // get pos
                fscanf( wfile, "%f", &obj->x );
                fscanf( wfile, "%f", &obj->y );

                // get comp
                for ( int j = 0; j < VC_COMP_LAST; j++ )
                {
                    fscanf( wfile, "%d", &obj->comp[ j ] );
                }

                if ( !vc_insert_object( world, obj ) )
                {
                    vc_free_object( obj );
                }
            }
        }

        if ( wfile ) fclose( wfile );
    }
    
    return world;
}

int vc_save_world( vc_world *world, char *outfile )
{
    if ( outfile == NULL || world == NULL )
        return 0;

    int status;
    FILE *wfile = fopen( outfile, "w" );

    if ( wfile != NULL )
    {
        /*
         * save world
         */
        
        fprintf( wfile, "%d\n", vc_get_object_count( world ) );

        vc_object *x = world->obj_list->root;

        while ( x )
        {
            vc_object *y = x;

            while ( y )
            {
                /*
                 * write object to file
                 */

                // not precise but good enough i guess
                fprintf( wfile, "%f\n", y->x );
                fprintf( wfile, "%f\n", y->y );

                for ( int i = 0; i < VC_COMP_LAST; i++ )
                {
                    fprintf( wfile, "%d\n", y->comp[ i ] );
                }

                y = y->above;
            }

            x = x->next;
        }

        status = 1;
    }
    else
    {
        status = 0;
    }

    if ( wfile ) fclose( wfile );
    return status;
}

/*
 * world getters and setters
 */

int vc_get_object_count( vc_world *world )
{
    if ( world == NULL )
        return -1;

    return world->obj_c;
}

/*
 * query
 */

vc_result *vc_query_point( vc_world *world, int x, int y )
{
    int point[] = { x, y };

    vc_result *result = ( vc_result * ) malloc( sizeof( vc_result ) );
    result->objects = NULL;
    result->current = ( vc_object * ) kd_search( world->obj_tree, point );
    result->length = 1;
    result->i = 0;

    return result;
}

vc_result *vc_query_range( vc_world *world, int x, int y, int r )
{
    int point[] = { x, y };

    vc_result *result = ( vc_result * ) malloc( sizeof( vc_result ) );
    result->objects = ( vc_object ** ) kd_query_range( world->obj_tree, point, r, &result->length );
    result->i = 0;
    result->current = result->objects == NULL ? NULL : result->objects[ 0 ];

    return result;
}

vc_result *vc_query_dim( vc_world *world, int x, int y, int w, int h )
{
    int point[] = { x, y };
    int dim[] = { w, h };

    vc_result *result = ( vc_result * ) malloc( sizeof( vc_result ) );
    result->objects = ( vc_object ** ) kd_query_dim( world->obj_tree, point, dim, &result->length );
    result->i = 0;
    result->current = result->objects == NULL ? NULL : result->objects[ 0 ];

    return result;
}

vc_object *vc_poll_result( vc_result *result )
{
    if ( result == NULL || result->current == NULL )
        return NULL;


    vc_object *object = result->current;

    // update current
    if ( object->above != NULL )
    {
        result->current = object->above;
    }
    else
    {
        result->i++;
        if ( result->i == result->length )
            result->current = NULL;
        else if ( result->objects != NULL )
            result->current = result->objects[ result->i ];
        else
            result->current = NULL;
    }

    return object;
}

/*
 * object create, get, and set
 */

vc_object *vc_new_object( float x, float y, enum vc_def_type def )
{
    vc_object *object = ( vc_object * ) malloc( sizeof( vc_object ) );
    object->x = x;
    object->y = y;

    object->world = NULL;
    object->next = NULL;
    object->prev = NULL;
    object->above = NULL;
    object->below = NULL;

    object->comp = ( int * ) malloc( sizeof( int ) * VC_COMP_LAST );

    // copy comp from def
    for ( int i = 0; i < VC_COMP_LAST; i++ )
        object->comp[ i ] = vc_get_def_comp( def, i );

    return object;
}

void vc_get_object_pos( vc_object *object, float *x, float *y )
{
    if ( object == NULL )
        return;

    if ( x ) *x = object->x;
    if ( y ) *y = object->y;
}

void vc_get_object_center( vc_object *object, float *x, float *y )
{
    if ( object == NULL )
        return;

    if ( x ) *x = object->x + 0.5f;
    if ( y ) *y = object->y + 0.5f;
}

int vc_set_object_pos( vc_object *object, float x, float y )
{
    if ( object == NULL )
        return 0;

    // currently objects cannot be set if it is attached to a world
    if ( object->world == NULL )
    {
        object->x = x;
        object->y = y;
        return 1;
    }
    else
    {
        return 0;
    }
}

int vc_add_object_pos( vc_object *object, float x, float y )
{
    if ( object == NULL )
        return 0;

    if ( object->world == NULL )
    {
        object->x += x;
        object->y += y;
        return 1;
    }
    else
    {
        return 0;
    }
}

int vc_get_object_comp( vc_object *object, enum vc_comp comp )
{
    if ( object == NULL )
        return -1;

    return object->comp[ comp ];
}

int vc_set_object_comp( vc_object *object, enum vc_comp comp, int value )
{
    if ( object == NULL )
        return 0;

    object->comp[ comp ] = value;
    return 1;
}

int vc_add_object_comp( vc_object *object, enum vc_comp comp, int value )
{
    if ( object == NULL )
        return 0;

    object->comp[ comp ] = value;
    return 1;
}

/*
 * world object attaching
 */

static int insert_object_util( vc_list *list, vc_object *node )
{
    if ( node == NULL )
        return 0;

    vc_object *root = list->root;

    if ( root == NULL )
    {
        root = node;
    }
    else
    {
        node->next = root;
        root->prev = node;
        root = node;
    }

    list->root = root;
    list->length++;
    return 1;
}

static int insert_object_above_util( vc_list *list, vc_object *a, vc_object *b )
{
    if ( a == NULL || b == NULL )
        return 0;

    if ( list->root == b )
        list->root = a;

    a->next = b->next;
    a->prev = b->prev;
    a->above = b;
    b->below = a;

    b->next = NULL;
    b->prev = NULL;

    if ( a->next )
        a->next->prev = a;

    if ( a->prev )
        a->prev->next = a;

    return 1;
}

// add object with no collision
int vc_insert_object_nc( vc_world *world, vc_object *object )
{
    if ( world == NULL || object == NULL || object->world != NULL )
        return 0;

    int point[] = { floor( object->x ), floor( object->y ) };
    vc_object *res = kd_insert( world->obj_tree, point, object );

    if ( res == object )
    {
        insert_object_util( world->obj_list, object );
        world->obj_c++;
        object->world = world;
        return 1;
    }
    else
    {
        return 0;
    }
}

// add object with no collision
int vc_insert_object( vc_world *world, vc_object *object )
{
    if ( world == NULL || object == NULL || object->world != NULL )
        return 0;

    int point[] = { floor( object->x ), floor( object->y ) };
    vc_object *res = kd_replace( world->obj_tree, point, object );

    if ( res == NULL )
    {
        return 0;
    }
    else if ( res == object )
    {
        insert_object_util( world->obj_list, object );
    }
    else
    {
        insert_object_above_util( world->obj_list, object, res );
    }

    world->obj_c++;
    object->world = world;
    return 1;
}

static int remove_object_util( vc_list *list, vc_object *node )
{
    if ( node == NULL || list->root == NULL )
        return 0;

    // if it is at very bottom
    if ( node->below == NULL )
    {
        // if there is no replacement node
        if ( node->above == NULL )
        {
            vc_object *root = list->root;

            // node is the root
            if ( node->prev == NULL )
            {
                root = node->next;

                // set new root prev
                if ( root )
                    root->prev = NULL;
            }
            else
            {
                node->prev->next = node->next;

                if ( node->next )
                    node->next->prev = node->prev;
            }

            list->root = root;
            list->length--;
        }
        // if there is a replacement node
        else
        {
            node->above->below = NULL;
            node->above->next = node->next;
            node->above->prev = node->prev;

            if ( node->next != NULL )
                node->next->prev = node->above;

            // check if we are at the root of list
            if ( node->prev != NULL )
                node->prev->next = node->above;
            else
                list->root = node->above;
        }
    }
    else
    {
        node->below->above = node->above;

        if ( node->above != NULL )
            node->above->below = node->below;
    }

    return 1;
}

int vc_remove_object( vc_world *world, vc_object *object )
{
    if ( world == NULL || object == NULL || object->world == NULL )
        return 0;

    if ( object->world != world )
        return 0;

    int point[] = { floor( object->x ), floor( object->y ) };

    // if there is a replacement node
    if ( object->above != NULL && object->below == NULL )
    {
        kd_replace( world->obj_tree, point, object->above );
        remove_object_util( world->obj_list, object );
    }
    // if there is no replacement node
    else if ( object->above == NULL && object->below == NULL )
    {
        kd_remove( world->obj_tree, point );
        remove_object_util( world->obj_list, object );
    }
    // if node is not in root list
    else if ( object->below != NULL )
    {
        remove_object_util( world->obj_list, object );
    }

    world->obj_c--;

    // reset object
    object->world = NULL;
    object->next = NULL;
    object->prev = NULL;
    object->above = NULL;
    object->below = NULL;

    return 1;
}

/*
 * general util function
 */

int vc_get_def_comp( enum vc_def_type def, enum vc_comp comp )
{
    return defs[ def ][ comp ];
}

/*
 * memory management
 */

static void free_list( vc_list *list )
{
    vc_object *x = list->root;
    while ( x )
    {
        vc_object *y = x->above;
        while ( y )
        {
            vc_object *temp = y;
            y = y->above;
            vc_free_object( temp );
        }

        vc_object *temp = x;
        x = x->next;
        vc_free_object( temp );
    }
    free( list );
}

void vc_free_world( vc_world *world )
{
    if ( world == NULL )
        return;

    kd_free( world->obj_tree );
    free_list( world->obj_list );
    free( world );
}

void vc_free_result( vc_result *result )
{
    if ( result == NULL )
        return;

    free( result->objects );
    free( result );
}

void vc_free_object( vc_object *object )
{
    if ( object == NULL )
        return;

    free( object->comp );
    free( object );
}
