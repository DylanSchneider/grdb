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


/* Place the code for your Dijkstra implementation in this file */
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
component_get_vertices(component_t c, /*int * count, */vertexid_t *list){
    ssize_t size, len;
    int readlen;
    char* buf;
    off_t off;
    vertexid_t v;
    int i;
    /*
    if (c->sv == NULL)
        size = 0;
    else
        size = schema_size(c->sv);
    
    readlen = sizeof(vertexid_t) + size;
    buf = malloc(readlen);
    
    *count = 0;
    for (off = 0;; off += readlen) {
        lseek(c->vfd, off, SEEK_SET);
        len = read(c->vfd, buf, readlen);
        
        if (len <= 0)
            break;
        (*count) ++;
    }
    free(buf);

    int malloc_size = ((*count) * sizeof(vertexid_t));
    printf("((%d))\n", malloc_size);
    list = malloc(malloc_size);*/
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

int get_weight_from_edge(component_t c, vertexid_t v1, vertexid_t v2, char attr_name[]){
    struct edge e;
    edge_t e1;
    int offset, weight;
    
    edge_init(&e);
    edge_set_vertices(&e, v1, v2);
    
    e1 = component_find_edge_by_ids(c, &e);
    
    if(e1 == NULL){
        printf("no edge between %llu and %llu\n", v1, v2);
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
    
    int number_of_vertices; //also used as size of vectors, upper limit on elements in the vectors is v=number of vertices
    
    /* variable sized vectors with count to keep track of element s*/
    vertexid_t *s_list;
    vertexid_t *parent_list;
    int s_list_count;
    int parent_list_count;
    
    /* fixed size vectors */
    vertexid_t *vertex_list;
    long long unsigned *cost_list;
    
    /* add file descriptors to component c */
    c->efd = edge_file_init(gno, cno);
    c->vfd = vertex_file_init(gno, cno);
    
    /* calculate max size for vectors, malloc */
    number_of_vertices = component_get_number_of_vertices(c);
    
    s_list = malloc(number_of_vertices * sizeof(vertexid_t));
    parent_list = malloc(number_of_vertices * sizeof(vertexid_t));
    vertex_list = malloc(number_of_vertices * sizeof(vertexid_t));
    cost_list = malloc(number_of_vertices * sizeof(long long unsigned));
    
    /*for(int i = 0; i < number_of_vertices; i++){
        s_list[i] = malloc_size
    }*/
    
    
    struct attribute *weight_attr;
    
    weight_attr = component_find_int_tuple(c->se->attrlist);
    if(!weight_attr){
        printf("No integer attributes found to use for weight\n");
        return -1;
    }
    
    component_get_vertices(c, vertex_list);
    
    for(int i = 0; i < number_of_vertices; i++){
        printf("[[%llu]]\n", vertex_list[i]);
    }
    
    
     

	/* Change this as needed */
	return (-1);
}
