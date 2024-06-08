#ifndef VECTOR_UTILS_H_
#define VECTOR_UTILS_H_

int get_random (int min, int max);
void vector_init_rand (int v[], long dim, int min, int max);
int cpy_buffer(int dest[], const int buf[], int buf_size, int* size);

#endif