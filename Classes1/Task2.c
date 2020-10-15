#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct{
    int size;
    int number_items;
    int reading_point;
    int writing_point;
    int* data;

    pthread_mutex_t index_mutex;
    pthread_cond_t size_cond;

}concurrent_buffer;

concurrent_buffer allocate_buffer(int size){
    concurrent_buffer buff;
    buff.data = (int*) malloc(size * sizeof(int));
    buff.size = size;
    buff.writing_point = 0;
    buff.reading_point = 0;
    buff.number_items = 0;
    pthread_mutex_init(&buff.index_mutex, NULL);
    pthread_cond_init(&buff.size_cond, NULL);
    return buff;
}

void deallocate_buffer(concurrent_buffer* buffer){
    free(buffer->data);
}

void read_from_concurrent_buffer(concurrent_buffer* b){
    pthread_mutex_lock(&b->index_mutex);
    if(b->number_items == 0){
        pthread_cond_wait(&b->size_cond, &b->index_mutex);
    }

    if(b->number_items == b->size){
        pthread_cond_broadcast(&b->size_cond);
    }

    b->number_items--;
    printf("%ld%s%d%s%d\n",pthread_self(), ": reading from index: ", b->reading_point, " value is: ", b->data[b->reading_point]);
    b->reading_point++;
    b->reading_point %= b->size;

    printf("%ld%s\n",pthread_self(), " read");

    pthread_mutex_unlock(&b->index_mutex);
}

void write_to_concurrent_buffer(concurrent_buffer* b){
    pthread_mutex_lock(&b->index_mutex);
    if(b->number_items == b->size){
        pthread_cond_wait(&b->size_cond, &b->index_mutex);
    }

    if(b->number_items == 0){
        pthread_cond_broadcast(&b->size_cond);
    }

    b->number_items++;
    int val = rand();
    b->data[b->writing_point] = val;
    printf("%ld%s%d%s%d\n",pthread_self() ,": writing to index: ", b->writing_point, " value is: ", b->data[b->writing_point]);
    b->writing_point++;
    b->writing_point %= b->size;

    printf("%ld%s\n",pthread_self() ," wrote");
    pthread_mutex_unlock(&b->index_mutex);
}

void* reader_thread(void* args){
    void** arg_arr = (void**) args;
    concurrent_buffer* b = (concurrent_buffer*) arg_arr[0];
    int* max_runs = (int*) arg_arr[1];

    for(int i = 0; i< *max_runs; i++){
        read_from_concurrent_buffer(b);
    }
}

void* writer_thread(void* args){
    void** arg_arr = (void**) args;
    concurrent_buffer* b = (concurrent_buffer*) arg_arr[0];
    int* max_runs = (int*) arg_arr[1];

    for(int i = 0; i< *max_runs; i++){
        write_to_concurrent_buffer(b);
    }    
}

int main(){
    srand(41);

    int num_readers = 2;
    int num_writers = 5;

    int reader_runs = 100;
    int writer_runs = 40; // num_readers * reader_run = writer_run * num_writers

    int buffer_size = 50;

    pthread_t writers[num_writers];
    pthread_t readers[num_readers];

    concurrent_buffer buffer = allocate_buffer(buffer_size);

    void* reader_args[] =  {(void*) &buffer, (void*) &reader_runs};
    void* writer_args[] =  {(void*) &buffer, (void*) &writer_runs};

    for(int i = 0; i<num_readers; i++){
        pthread_create(&readers[i], NULL, reader_thread, reader_args);
    }

    for(int i = 0; i<num_writers; i++){
        pthread_create(&writers[i], NULL, writer_thread, writer_args);
    }

    for(int i = 0; i<num_readers; i++){
        pthread_join(readers[i], NULL);
    }

    for(int i = 0; i<num_writers; i++){
        pthread_join(writers[i], NULL);
    }

    deallocate_buffer(&buffer);

    return 0;
}