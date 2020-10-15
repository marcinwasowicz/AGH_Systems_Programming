#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct{
    int* data;
    int size;
    int reading_point;
    int writing_point;
    int array_idx;
    int number_items;

    pthread_mutex_t index_mutex;
}concurrent_buffer;

concurrent_buffer allocate_buffer(int size, int array_idx){
    concurrent_buffer buffer;
    buffer.size = size;
    buffer.data = (int*) malloc(size*sizeof(int));
    buffer.reading_point = 0;
    buffer.writing_point = 0;
    buffer.array_idx = array_idx;
    buffer.number_items = 0;

    pthread_mutex_init(&buffer.index_mutex, NULL);
    return buffer;
}

void deallocate_buffer(concurrent_buffer* buffer){
    free(buffer->data);
}

bool write_to_buffer(concurrent_buffer* buffer){
    if(pthread_mutex_trylock(&buffer->index_mutex) != 0){
        return false;
    }

    if(buffer->number_items == buffer->size){
        pthread_mutex_unlock(&buffer->index_mutex);
        return false;
    }

    buffer->number_items++;
    int value = rand();
    buffer->data[buffer->writing_point] = value;
    printf("thread of id: %ld wrote to buffer %d value %d at index %d\n",pthread_self(),buffer->array_idx, value, buffer->writing_point);
    buffer->writing_point++;
    buffer->writing_point %= buffer->size;
    pthread_mutex_unlock(&buffer->index_mutex);
    return true;
}

bool read_from_buffer(concurrent_buffer* buffer){
    if(pthread_mutex_trylock(&buffer->index_mutex) != 0){
        return false;
    }

    if(buffer->number_items == 0){
        pthread_mutex_unlock(&buffer->index_mutex);
        return false;
    }

    buffer->number_items--;
    printf("thread of id: %ld reading from buffer %d value %d at index %d\n", pthread_self(), buffer->array_idx, buffer->data[buffer->reading_point], buffer->reading_point);
    buffer->reading_point++;
    buffer->reading_point %= buffer->size;
    pthread_mutex_unlock(&buffer->index_mutex);
    return true;
}

void* writer_thread(void* args){
    void** args_arr = (void**) args;
    concurrent_buffer* buffer_arr = (concurrent_buffer*) args_arr[0];
    int* max_runs = (int*) args_arr[1];
    int* array_size = (int*) args_arr[2];

    for(int i = 0; i<*max_runs;){
        if(write_to_buffer(&buffer_arr[rand() % *array_size])){
            i++;
        }
    }
}


void* reader_thread(void* args){
    void** args_arr = (void**) args;
    concurrent_buffer* buffer_arr = (concurrent_buffer*) args_arr[0];
    int* max_runs = (int*) args_arr[1];
    int* array_size = (int*) args_arr[2];

    for(int i = 0; i<*max_runs;){
        if(read_from_buffer(&buffer_arr[rand() % *array_size])){
            i++;
            continue;
        }
    }
}


int main(){
    int num_writers = 10;
    int num_readers = 20;

    int writer_runs = 6;
    int reader_runs = 3;

    pthread_t writers[num_writers];
    pthread_t readers[num_readers];

    int buffer_size = 6;
    int num_buffers = 10;

    concurrent_buffer buffer_array[num_buffers];

    void* writer_args[] = {(void*) buffer_array, (void*) &writer_runs, (void*) &num_buffers};
    void* reader_args[] = {(void*) buffer_array, (void*) &reader_runs, (void*) &num_buffers};

    for(int i = 0; i<num_buffers; i++){
        buffer_array[i] = allocate_buffer(buffer_size, i);
    }

    for(int i = 0; i<num_writers; i++){
        pthread_create(&writers[i], NULL, writer_thread, writer_args);
    }

    for(int i = 0; i<num_readers; i++){
        pthread_create(&readers[i], NULL, reader_thread, reader_args);
    }

    for(int i = 0; i<num_readers; i++){
        pthread_join(readers[i], NULL);
    }

    for(int i = 0; i<num_writers; i++){
        pthread_join(writers[i], NULL);
    }

    for(int i = 0; i<num_buffers; i++){
        deallocate_buffer(&buffer_array[i]);
    }

    return 0;
}