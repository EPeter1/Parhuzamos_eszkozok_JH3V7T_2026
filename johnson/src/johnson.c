#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "johnson.h"
#include "opencl.h"

void parse_arguments(int argc, char* argv[], int* vertices) {

    int option;
    bool is_option_used = false;

    while ((option = getopt(argc, argv, "v:")) != -1) {
        switch (option) {
            case 'v':
                *vertices = atoi(optarg);
                is_option_used = true;
                break;
            default:
                fprintf(stderr, "Usage: %s -v <number_of_vertices>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (!is_option_used) {
        fprintf(stderr, "Error: The number of vertices must be specified!\n");
        fprintf(stderr, "Usage: %s -v <number_of_vertices>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (*vertices <= 0) {
        fprintf(stderr, "The number of vertices must be a positive integer!\n");
        exit(EXIT_FAILURE);
    }
}

int generate_number(int lower, int upper) {

    return rand() % (upper - lower + 1) + lower;
}

bool edge_exists(Edge* edge_list, int edges, int source, int destination) {

    for (int i = 0; i < edges; i++) {
        if (edge_list[i].source == source && edge_list[i].destination == destination) {
            return true;
        }
    }

    return false;
}

Edge* generate_edge_list(int vertices, int edges) {

    Edge* edge_list = malloc(edges * sizeof(Edge));

    for (int i = 0; i < edges; i++) {
        int new_source;
        int new_destination;

        // Önhurok és ismétlődés elkerülése
        do {
            new_source = generate_number(0, vertices - 1);
            new_destination = generate_number(0, vertices - 1);
        } while (new_source == new_destination || edge_exists(edge_list, i, new_source, new_destination));

        edge_list[i].source = new_source;
        edge_list[i].destination = new_destination;
        edge_list[i].weight = generate_number(1, 10);
        // edge_list[i].weight = generate_number(-10, 10);  // Negatív kör ellenőrzése
    }

    return edge_list;
}

void convert_edge_list_to_csr_format(Edge* edge_list, Graph* graph) {

    int vertices = graph->vertices;
    int edges = graph->edges;
    int* row_pointer = graph->row_pointer;
    int* column_index = graph->column_index;
    int* weights = graph->weights;

    memset(row_pointer, 0, (vertices + 1) * sizeof(int));

    for (int i = 0; i < edges; i++) {
        int source = edge_list[i].source;
        row_pointer[source + 1]++;
    }

    for (int i = 1; i <= vertices; i++) {
        row_pointer[i] += row_pointer[i - 1];
    }

    int* offset = malloc(vertices * sizeof(int));
    memcpy(offset, row_pointer, vertices * sizeof(int));

    for (int i = 0; i < edges; i++) {
        int source = edge_list[i].source;
        int destination = offset[source]++;

        column_index[destination] = edge_list[i].destination;
        weights[destination] = edge_list[i].weight;
    }

    free(offset);
}

Graph* create_graph(int vertices, int edges) {

    Graph* graph = malloc(sizeof(Graph));

    graph->vertices = vertices;
    graph->edges = edges;
    graph->row_pointer = malloc((vertices + 1) * sizeof(int));
    graph->column_index = malloc(edges * sizeof(int));
    graph->weights = malloc(edges * sizeof(int));

    return graph;
}

Graph* create_graph_from_edge_list(Edge* edge_list, int vertices, int edges) {

    Graph* graph = create_graph(vertices, edges);
    convert_edge_list_to_csr_format(edge_list, graph);

    return graph;
}

Graph* create_extended_graph(Graph* graph) {

    int vertices = graph->vertices;
    int edges = graph->edges;

    Graph* extended_graph = create_graph(vertices + 1, edges + vertices);
    int* row_pointer = extended_graph->row_pointer;
    int* column_index = extended_graph->column_index;
    int* weights = extended_graph->weights;

    memcpy(row_pointer, graph->row_pointer, (vertices + 1) * sizeof(int));
    memcpy(column_index, graph->column_index, edges * sizeof(int));
    memcpy(weights, graph->weights, edges * sizeof(int));

    for (int i = 0; i < vertices; i++) {
        column_index[edges + i] = i;
        weights[edges + i] = 0;
    }

    row_pointer[vertices] = edges;
    row_pointer[vertices + 1] = edges + vertices;

    return extended_graph;
}

void destroy_graph(Graph* graph) {

    if (graph == NULL) {
        return;
    }

    free(graph->row_pointer);
    free(graph->column_index);
    free(graph->weights);
    
    free(graph);
}

void initialize_single_source(Graph* graph, int source, int* distance) {

    int vertices = graph->vertices;

    for (int i = 0; i < vertices; i++) {
        distance[i] = INF;
    }

    distance[source] = 0;
}

void relax_edges(int source, int destination, int weight, int* distance) {

    if ((distance[source] != INF) && (distance[destination] > distance[source] + weight)) {
        distance[destination] = distance[source] + weight;
    }
}

bool bellman_ford(Graph* graph, int source, int* distance) {

    int vertices = graph->vertices;
    int* row_pointer = graph->row_pointer;
    int* column_index = graph->column_index;
    int* weights = graph->weights;

    initialize_single_source(graph, source, distance);

    for (int i = 1; i < vertices; i++) {

        for (int j = 0; j < vertices; j++) {
            
            for (int k = row_pointer[j]; k < row_pointer[j + 1]; k++) {

                int neighbor = column_index[k];
                int weight = weights[k];
                relax_edges(j, neighbor, weight, distance);
            }
        }
        
    }

    for (int i = 0; i < vertices; i++) {

        for (int j = row_pointer[i]; j < row_pointer[i + 1]; j++) {

            int neighbor = column_index[j];
            int weight = weights[j];

            if ((distance[i] != INF) && (distance[neighbor] > distance[i] + weight)) {
                printf("Negative cycle detected, algorithm terminated!\n");
                return false;
            }
        }
    }

    return true;
}

void print_all_shortest_paths(int* paths, int vertices) {

    int vertices_to_display = 20;

    printf("Shortest paths:\n\n");
    printf("From\\To");

    for (int i = 0; i < vertices_to_display; i++) {
        printf("%5d", i + 1);
    }
    printf("\n");

    for (int i = 0; i < vertices_to_display; i++) {
        printf("%5d  ", i + 1);

        for (int j = 0; j < vertices_to_display; j++) {
            int value = paths[i * vertices + j];

            if (value == INF) {
                printf("%5s", "INF");
            }
            else {
                printf("%5d", value);
            }
        }
        printf("\n");
    }
}

void johnson(Graph* graph, OpenCLContext* context) {

    int vertices = graph->vertices;
    int edges = graph->edges;
    int* potential = malloc((vertices + 1) * sizeof(int));
    int* paths = malloc(vertices * vertices * sizeof(int));
    bool* visited = calloc(vertices * vertices, sizeof(bool));
    Graph* extended_graph = create_extended_graph(graph);

    if (!bellman_ford(extended_graph, vertices, potential)) {

        free(potential);
        free(paths);
        free(visited);
        destroy_graph(extended_graph);

        return;
    }
    
    cl_mem device_row_pointer = create_buffer(context, "read", (vertices + 1) * sizeof(int));
    cl_mem device_column_index = create_buffer(context, "read", edges * sizeof(int));
    cl_mem device_weights = create_buffer(context, "read_write", edges * sizeof(int));
    cl_mem device_potential = create_buffer(context, "read", (vertices + 1) * sizeof(int));
    cl_mem device_paths = create_buffer(context, "write", vertices * vertices * sizeof(int));
    cl_mem device_visited = create_buffer(context, "read_write", vertices * vertices * sizeof(bool));

    cl_kernel kernel_reweight = build_kernel(context, "kernels/reweight.cl", "reweight_graph");
    cl_kernel kernel_dijkstra = build_kernel(context, "kernels/dijkstra.cl", "dijkstra");

    set_kernel_argument(kernel_reweight, 0, sizeof(cl_mem), &device_row_pointer);
    set_kernel_argument(kernel_reweight, 1, sizeof(cl_mem), &device_column_index);
    set_kernel_argument(kernel_reweight, 2, sizeof(cl_mem), &device_weights);
    set_kernel_argument(kernel_reweight, 3, sizeof(cl_mem), &device_potential);
    set_kernel_argument(kernel_reweight, 4, sizeof(int), &vertices);

    set_kernel_argument(kernel_dijkstra, 0, sizeof(cl_mem), &device_row_pointer);
    set_kernel_argument(kernel_dijkstra, 1, sizeof(cl_mem), &device_column_index);
    set_kernel_argument(kernel_dijkstra, 2, sizeof(cl_mem), &device_weights);
    set_kernel_argument(kernel_dijkstra, 3, sizeof(cl_mem), &device_potential);
    set_kernel_argument(kernel_dijkstra, 4, sizeof(cl_mem), &device_paths);
    set_kernel_argument(kernel_dijkstra, 5, sizeof(cl_mem), &device_visited);
    set_kernel_argument(kernel_dijkstra, 6, sizeof(int), &vertices);

    write_to_device(context, device_row_pointer, (vertices + 1) * sizeof(int), graph->row_pointer);
    write_to_device(context, device_column_index, edges * sizeof(int), graph->column_index);
    write_to_device(context, device_weights, edges * sizeof(int), graph->weights);
    write_to_device(context, device_potential, (vertices + 1) * sizeof(int), potential);
    write_to_device(context, device_paths, vertices * vertices * sizeof(int), paths);
    write_to_device(context, device_visited, vertices * vertices * sizeof(bool), visited);

    size_t local_work_size = 16;
    size_t work_groups = (vertices + local_work_size - 1) / local_work_size;
    size_t global_work_size = work_groups * local_work_size;

    start_kernel(context, kernel_reweight, global_work_size, local_work_size);
    start_kernel(context, kernel_dijkstra, global_work_size, local_work_size);

    read_from_device(context, device_paths, vertices * vertices * sizeof(int), paths);
    print_all_shortest_paths(paths, vertices);

    clReleaseMemObject(device_row_pointer);
    clReleaseMemObject(device_column_index);
    clReleaseMemObject(device_weights);
    clReleaseMemObject(device_potential);
    clReleaseMemObject(device_paths);
    clReleaseMemObject(device_visited);

    free(potential);
    free(paths);
    free(visited);
    destroy_graph(extended_graph);
}

void get_execution_time(clock_t start_time, clock_t end_time) {

    double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("\nExecution time: %.4lf sec", execution_time);
}
