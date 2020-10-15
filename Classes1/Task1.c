#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct{
    int data;
    int readers_count;

    pthread_mutex_t writer_mutex;
    pthread_mutex_t reader_mutex;
}concurrent_object;

concurrent_object allocate_concurrent_object(int initial_data){
    concurrent_object object;
    object.data = initial_data;
    object.readers_count = 0;

    pthread_mutex_init(&object.writer_mutex, NULL);
    pthread_mutex_init(&object.reader_mutex, NULL);

    return object;
}

int write_to_concurrent_object(concurrent_object* object){
    pthread_mutex_lock(&object->writer_mutex);
    printf("writer of id: %ld got access to object\n", pthread_self());
    object->data = rand();
    printf("writer of id: %ld wrote to object: %d\n", pthread_self(), object->data);
    pthread_mutex_unlock(&object->writer_mutex);
}

void read_from_concurrent_object(concurrent_object* object){
    pthread_mutex_lock(&object->reader_mutex);
    object->readers_count++;
    if(object->readers_count == 1){
        pthread_mutex_lock(&object->writer_mutex);
    }
    pthread_mutex_unlock(&object->reader_mutex);
    printf("reader of id: %ld got access to object\n", pthread_self());
    printf("reader of id: %ld read from object: %d\n", pthread_self(), object->data);
    pthread_mutex_lock(&object->reader_mutex);
    object->readers_count--;
    if(object->readers_count == 0){
        pthread_mutex_unlock(&object->writer_mutex);
    }
    pthread_mutex_unlock(&object->reader_mutex);
}

void* writer_thread(void* args){
    void** args_arr = (void**) args;
    concurrent_object* object = (concurrent_object*) args_arr[0];
    int* writer_runs = (int*) args_arr[1];
    for(int i = 0; i<*writer_runs; i++){
        write_to_concurrent_object(object);
    }
}

void* reader_thread(void* args){
    void** args_arr = (void**) args;
    concurrent_object* object = (concurrent_object*) args_arr[0];
    int* reader_runs = (int*) args_arr[1];
    for(int i = 0; i<*reader_runs; i++){
        read_from_concurrent_object(object);
    }
}

int main(){
    srand(41);

    int num_readers = 2;
    int num_writers = 5;

    int reader_runs = 100;
    int writer_runs = 100;

    int initial_data = 0;

    pthread_t writers[num_writers];
    pthread_t readers[num_readers];

    concurrent_object object = allocate_concurrent_object(initial_data);

    void* reader_args[] =  {(void*) &object, (void*) &reader_runs};
    void* writer_args[] =  {(void*) &object, (void*) &writer_runs};

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

    return 0;
}