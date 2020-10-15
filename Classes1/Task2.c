#include <pthread.h>
#include <stdio.h>

typedef struct{
    int size;
    int number_items;
    int reading_point;
    int writing_point;

    pthread_mutex_t index_mutex;
    pthread_cond_t size_cond;

}buffer;

void read_buffer(buffer* b){
    pthread_mutex_lock(&b->index_mutex);
    if(b->number_items == 0){
        pthread_cond_wait(&b->size_cond, &b->index_mutex);
    }

    if(b->number_items == b->size){
        pthread_cond_broadcast(&b->size_cond);
    }

    b->number_items--;
    printf("%ld%s%d\n",pthread_self(), ": reading from index: ", b->reading_point);
    b->reading_point++;
    b->reading_point %= b->size;

    printf("%ld%s\n",pthread_self(), " read");

    pthread_mutex_unlock(&b->index_mutex);
}

void write_buffer(buffer* b){
    pthread_mutex_lock(&b->index_mutex);
    if(b->number_items == b->size){
        pthread_cond_wait(&b->size_cond, &b->index_mutex);
    }

    if(b->number_items == 0){
        pthread_cond_broadcast(&b->size_cond);
    }

    b->number_items++;
    printf("%ld%s%d\n",pthread_self() ,": writing to index: ", b->writing_point);
    b->writing_point++;
    b->writing_point %= b->size;

    printf("%ld%s\n",pthread_self() ," wrote");
    pthread_mutex_unlock(&b->index_mutex);
}

buffer allocate_buffer(int size){
    buffer buff;
    buff.size = size;
    buff.writing_point = 0;
    buff.reading_point = 0;
    buff.number_items = 0;
    pthread_mutex_init(&buff.index_mutex, NULL);
    pthread_cond_init(&buff.size_cond, NULL);
    return buff;
}

void* reader_thread(void* args){
    void** arg_arr = (void**) args;
    buffer* b = (buffer*) arg_arr[0];
    int* max_runs = (int*) arg_arr[1];

    for(int i = 0; i< *max_runs; i++){
        read_buffer(b);
    }
}

void* writer_thread(void* args){
    void** arg_arr = (void**) args;
    buffer* b = (buffer*) arg_arr[0];
    int* max_runs = (int*) arg_arr[1];

    for(int i = 0; i< *max_runs; i++){
        write_buffer(b);
    }    
}

int main(){
    int max_runs = 1000;
    int num_threads = 10000;
    buffer buff = allocate_buffer(1);

    void* args_arr[] = {(void*) &buff, (void *) &max_runs};

    pthread_t threads[num_threads];

    for(int i = 0; i<num_threads/2; i++){
        pthread_create(&threads[i], NULL, writer_thread, args_arr);
    }

    for(int i = num_threads/2; i<num_threads; i++){
        pthread_create(&threads[i], NULL, reader_thread, args_arr);
    }

    for(int i = 0; i<num_threads; i++){
        pthread_join(threads[i], NULL);
    }

    return 0;
}