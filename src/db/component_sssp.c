#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include "config.h"
#include "cli.h"
#include "graph.h"
#define INT_type 4
#define inf INT_MAX
#define min(x, y) (((x) < (y)) ? (x) : (y))


/* Place the code for your Dijkstra implementation in this file */
int
get_index_from_id(vertexid_t id, vertexid_t* list,int count){
    for(int i = 0; i < count; i++){
        if(list[i] == id){
            return i;
        }
    }
    return -1;
}


int
component_get_number_of_vertices(component_t c){
    ssize_t size, len;
    char* buf;
    off_t off;
    int readlen, count;
    
    if (c->sv == NULL)
        size = 0;
    else
        size = schema_size(c->sv);
    
    readlen = sizeof(vertexid_t) + size;
    buf = malloc(readlen);
    
    count = 0;
    for (off = 0;; off += readlen) {
        lseek(c->vfd, off, SEEK_SET);
        len = read(c->vfd, buf, readlen);
        
        if (len <= 0)
            break;
        (count) ++;
    }
    free(buf);
    return count;
}

void
component_get_vertices(component_t c, vertexid_t *list){
    ssize_t size, len;
    int readlen;
    char* buf;
    off_t off;
    vertexid_t v;
    int i;

    if (c->sv == NULL)
        size = 0;
    else
        size = schema_size(c->sv);
    
    readlen = sizeof(vertexid_t) + size;
    buf = malloc(readlen);
    
    for (i = 0,off = 0;; i++, off += readlen) {
        lseek(c->vfd, off, SEEK_SET);
        len = read(c->vfd, buf, readlen);
        if (len <= 0)
            break;
        
        v = *((vertexid_t *) buf);
        list[i] = v;
    }
    free(buf);
}

attribute_t
component_find_int_tuple(attribute_t a)
{
    attribute_t attr;
    
    assert (a != NULL);
    
    for(attr = a; attr != NULL; attr = attr->next){
        if(attr->bt == INT_type)
            return attr;
    }
    
    return NULL;
}

int
get_weight_from_edge(component_t c, vertexid_t v1, vertexid_t v2, char attr_name[]){
    struct edge e;
    edge_t e1;
    int offset, weight;
    
    edge_init(&e);
    edge_set_vertices(&e, v1, v2);
    
    e1 = component_find_edge_by_ids(c, &e);
    
    if(e1 == NULL){
        return inf;
    }
    
    offset = tuple_get_offset(e1->tuple, attr_name);
    weight = tuple_get_int(e1->tuple->buf + offset);
    return weight;
}

int
component_sssp(
        component_t c,
        vertexid_t start,
        vertexid_t end,
        int *n,
        int *total_weight,
        vertexid_t **path)
{
    
    if(start != 1){
        printf("This Dijkstra's implementation only works with starting vertex = 1\n");
        return -1;
    }
    
    int number_of_vertices; //also used as size of vectors, upper limit on elements in the vectors is v=number of vertices
    
    /* variable sized vectors with count to keep track of element s*/
    vertexid_t *s_list;
    vertexid_t *sssp_list;
    int s_list_count;
    int sssp_list_count;
    
    /* fixed size vectors */
    vertexid_t *vertex_list;
    int *cost_list;
    vertexid_t *parent_list;
    
    /* add file descriptors to component c */
    c->efd = edge_file_init(gno, cno);
    c->vfd = vertex_file_init(gno, cno);
    
    /* check for INT attribute type */
    attribute_t weight_attr;
    weight_attr = component_find_int_tuple(c->se->attrlist);
    if(!weight_attr){
        printf("No integer attributes found to use for weight\n");
        return -1;
    }
    
    
    /* calculate max size for vectors, malloc */
    number_of_vertices = component_get_number_of_vertices(c);
    
    /* initialize vectors */
    parent_list = malloc(number_of_vertices * sizeof(vertexid_t));
    vertex_list = malloc(number_of_vertices * sizeof(vertexid_t));
    cost_list = malloc(number_of_vertices * sizeof(int));
    sssp_list = malloc(number_of_vertices * sizeof(int));
    
    
    component_get_vertices(c, vertex_list);
    for(int i = 0; i < number_of_vertices; i++){
        cost_list[i] = inf;
        parent_list[i] = inf;
    }
    
    /* traversal variables */
    int start_index;
    int end_index;
    int min_index;
    int finished_start;
    int min;
    int in_vs;
    vertexid_t w, v;
    
    finished_start = 0;
    
    start_index = -1;
    end_index = -1;
    for(int k = 0; k < number_of_vertices; k++){
        if(vertex_list[k] == start)
            start_index = k;
        if(vertex_list[k] == end)
            end_index = k;
    }
    
    /* set all adjacents of start's parents to start */
    for(int i = 1; i < number_of_vertices; i++){
        int temp_weight = get_weight_from_edge(c, start, vertex_list[i], weight_attr->name);
        if(temp_weight != inf){
            parent_list[i] = start;
        }
        
    }
    
    /* begin traversal */
    for(int i = 1; i < number_of_vertices; i++){
        /* initialize s list with starting vertex */
        s_list = malloc(number_of_vertices * sizeof(vertexid_t));
        s_list[0] = start;
        s_list_count = 1;
        
        cost_list[i] = get_weight_from_edge(c, start, vertex_list[i], weight_attr->name);
        /* choose w in V-S such that D[w] is min */
        min = inf;
        for(int j = 0; j < number_of_vertices; j++){
            /* check if w in V-S */
            in_vs = 1;
            for(int k = 0; k < s_list_count; k++){
                if(vertex_list[j] == s_list[k])
                    in_vs = 0;
            }
            /* calculate min in V-S */
            if(in_vs){
                if(cost_list[j] < min){
                    min_index = j;
                }
            }
            else{
                continue;
            }
            
            /* add vertexid w to S */
            w = j;
            s_list[s_list_count] = vertex_list[w];
            s_list_count++;
            
            /* check if this edge is on shortest path */
            for(int l = 0; l < number_of_vertices; l++){
                /* check if v in V-S */
                v = l;
                in_vs = 1;
                for(int m = 0; m < s_list_count; m++){
                    if(vertex_list[v] == s_list[m])
                        in_vs = 0;
                }
                /* set cost matrix and check if shortest path is found and add parent */
                if(in_vs){
                    int c_wv = get_weight_from_edge(c, vertex_list[w], vertex_list[v], weight_attr->name);
                    if(cost_list[w] == inf || c_wv == inf){
                        /* if D[w] or C[w,v] are inf, then the < comparison doesnt need to be made*/
                        cost_list[v] = min(cost_list[v], inf);
                        
                    }
                    else{
                        /* neither are inf, can do addition and comparison */
                        if(cost_list[w] + c_wv < cost_list[v]){
                            parent_list[v] = vertex_list[w];
                            cost_list[v] = cost_list[w] + c_wv;
                        }
                    }
                }
            }
        }
    }
    
    cost_list[start_index] = 0;
#if _DEBUG
    printf("Cost List: [");
    for(int n = 0; n < number_of_vertices; n++){
        if(n != (number_of_vertices - 1))
            printf("%d,", cost_list[n]);
        else
            printf("%d", cost_list[n]);
    }
    printf("]\n");
    parent_list[start_index] = 0;
    printf("Parent List : [");
    for(int n = 0; n < number_of_vertices; n++){
        if(n != (number_of_vertices - 1))
            printf("%llu,", parent_list[n]);
        else
            printf("%llu", parent_list[n]);
    }
    printf("]\n");
#endif
    
    sssp_list[0] = end;
    sssp_list_count = 1;
    
    
    for(vertexid_t temp = parent_list[end_index]; temp != start; temp = parent_list[get_index_from_id(temp, vertex_list, number_of_vertices)]){
        sssp_list[sssp_list_count] = temp;
        sssp_list_count++;
        
    }
    
    sssp_list[sssp_list_count] = start;
    sssp_list_count++;
    
    printf("The shortest path between %llu and %llu has a cost of: %d\n", start, end, cost_list[end_index]);
    printf("SSSP List : [");
    for(int n = sssp_list_count-1; n >= 0; n--){
        if(n != 0)
            printf("%llu,", sssp_list[n]);
        else
            printf("%llu", sssp_list[n]);
    }
    printf("]\n");
    
    /* free vectors */
    free(parent_list);
    free(vertex_list);
    free(cost_list);
    free(sssp_list);
    free(s_list);

	/* Change this as needed */
	return (-1);
}
