#define INF INT_MAX

inline void initialize_single_source(int vertices, int source, __global int* distance) {
    
    for (int i = 0; i < vertices; i++) {
        distance[i] = INF;
    }

    distance[source] = 0;
}

inline void relax_edges(int source, int destination, int weight, __global int* distance) {

    if ((distance[source] != INF) && (distance[destination] > distance[source] + weight)) {
        distance[destination] = distance[source] + weight;
    }
}

__kernel void dijkstra(
    __global const int* row_pointer,
    __global const int* column_index,
    __global const int* weights,
    __global const int* potential,
    __global int* paths,
    __global bool* visited,
    const int vertices)
{
    int source = get_global_id(0);

    if (source >= vertices) {
        return;
    }

    __global int* distance = &paths[source * vertices];
    __global bool* visited_row = &visited[source * vertices];
    
    initialize_single_source(vertices, source, distance);

    for (int i = 0; i < vertices - 1; i++) {

        int min = INF;
        int from = -1;

        for (int j = 0; j < vertices; j++) {

            if (!visited_row[j] && distance[j] <= min) {
                min = distance[j];
                from = j;
            }
        }

        if (from == -1) {
            break;
        }
        visited_row[from] = true;

        for (int j = row_pointer[from]; j < row_pointer[from + 1]; j++) {
            int to = column_index[j];
            int weight = weights[j];

            if (!visited_row[to]) {
                relax_edges(from, to, weight, distance);
            }
        }
    }

    for (int i = 0; i < vertices; i++) {
        if (distance[i] != INF) {
            distance[i] = distance[i] - potential[source] + potential[i];
        }
    }
}
