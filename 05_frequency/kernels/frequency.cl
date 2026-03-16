__kernel void get_frequency(
    __global const int* input,
    __global int* frequency,
    const int offset,
    const int range,
    const int input_size)
{
    int id = get_global_id(0);

    if (id >= input_size) {
        return;
    }

    int value = input[id];
    int index = value - offset;

    if (index >= 0 && index < range) {
        atomic_inc(&frequency[index]);
    }
}
