#ifndef JOHNSON_H
#define JOHNSON_H

#include <limits.h>
#include <stdbool.h>
#include <time.h>

#include "opencl.h"

#define INF INT_MAX

typedef struct Edge {
    int source;
    int destination;
    int weight;
} Edge;

typedef struct Graph {
    int vertices;
    int edges;
    int* row_pointer;
    int* column_index;
    int* weights;
} Graph;

void parse_arguments(int argc, char* argv[], int* vertices);
int generate_number(int lower, int upper);
bool edge_exists(Edge* edge_list, int edges, int source, int destination);
Edge* generate_edge_list(int vertices, int edges);
void convert_edge_list_to_csr_format(Edge* edge_list, Graph* graph);
Graph* create_graph(int vertices, int edges);
Graph* create_graph_from_edge_list(Edge* edge_list, int vertices, int edges);
Graph* create_extended_graph(Graph* graph);
void destroy_graph(Graph* graph);
void initialize_single_source(Graph* graph, int source, int* distance);
void relax_edges(int source, int destination, int weight, int* distance);
bool bellman_ford(Graph* graph, int source, int* distance);
void print_all_shortest_paths(int* paths, int vertices);
void johnson(Graph* graph, OpenCLContext* context);
void get_execution_time(clock_t start_time, clock_t end_time);

#endif // JOHNSON_H
