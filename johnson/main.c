#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "johnson.h"
#include "opencl.h"

int main(int argc, char* argv[]) {

    int vertices = 1000;
    parse_arguments(argc, argv, &vertices);
    int edges = 2 * vertices;

    srand(time(0));
    Edge* edge_list = generate_edge_list(vertices, edges);
    Graph* graph = create_graph_from_edge_list(edge_list, vertices, edges);
    OpenCLContext context = initialize_opencl();

    printf("----------Johnson algorithm----------\n");
    printf("Input size (V, E): (%d, %d)\n\n", vertices, edges);

    clock_t start_time = clock();
    johnson(graph, &context);
    clock_t end_time = clock();
    get_execution_time(start_time, end_time);

    free(edge_list);
    destroy_graph(graph);
    destroy_context(&context);
}
