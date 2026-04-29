__kernel void reweight_graph(
    __global const int* row_pointer,
    __global const int* column_index,
    __global int* weights,
    __global const int* potential,
    const int vertices) 
{
    int id = get_global_id(0);

    if (id < vertices) {
        for (int i = row_pointer[id]; i < row_pointer[id + 1]; i++) {
            int neighbor = column_index[i];
            int original_weight = weights[i];

            weights[i] = original_weight + potential[id] - potential[neighbor];
        }
    }
}
