#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct{
    int data;
    int readers_count;
    int array_idx;

    pthread_mutex_t readers_mutex;
    pthread_mutex_t writers_mutex;
}concurrent_object;

concurrent_object allocate_object(int initial_data, int array_idx){
    concurrent_object object;
    object.data = initial_data;
    object.readers_count = 0;
    object.array_idx = array_idx;
    pthread_mutex_init(&object.readers_mutex, NULL);
    pthread_mutex_init(&object.writers_mutex, NULL);
    return object;
}

bool write_to_object(concurrent_object* object){
    if(pthread_mutex_trylock(&object->writers_mutex) != 0){
        return false;
    }
    int value = rand();
    printf("thread of id: %ld is writing value %d to object at index %d\n", pthread_self(), value, object->array_idx);
    object->data = value;
    pthread_mutex_unlock(&object->writers_mutex);
    return true;
}

bool read_from_object(concurrent_object* object){
    pthread_mutex_lock(&object->readers_mutex);
    if(object->readers_count == 0 && pthread_mutex_trylock(&object->writers_mutex) != 0){
        pthread_mutex_unlock(&object->readers_mutex);
        return false;
    }
    object->readers_count++;
    pthread_mutex_unlock(&object->readers_mutex);

    printf("thread of id: %ld is reading value %d from object at index %d\n", pthread_self(), object->data, object->array_idx);

    pthread_mutex_lock(&object->readers_mutex);
    object->readers_count--;
    if(object->readers_count == 0){
        pthread_mutex_unlock(&object->writers_mutex);
    }
    pthread_mutex_unlock(&object->readers_mutex);
    return true;
}

void* writer_thread(void* args){
    void** args_arr = (void**) args;
    concurrent_object* object_array = (concurrent_object*) args_arr[0];
    int* array_size = (int*) args_arr[1];
    int* max_runs = (int*) args_arr[2];

    for(int i = 0; i<*max_runs;){
        if(write_to_object(&object_array[rand() % *array_size])){
            i++;
        }
    }
}

void* reader_thread(void* args){
    void** args_arr = (void**) args;
    concurrent_object* object_array = (concurrent_object*) args_arr[0];
    int* array_size = (int*) args_arr[1];
    int* max_runs = (int*) args_arr[2];

    for(int i = 0; i<*max_runs;){
        if(read_from_object(&object_array[rand() % *array_size])){
            i++;
        }
    }
}

int main(){
    int num_writers = 500;
    int num_readers = 500;

    int writer_turns = 15;
    int reader_turns = 20;

    int num_objects = 50;
    int initial_data = 0;

    concurrent_object objects[num_objects];

    pthread_t writers[num_writers];
    pthread_t readers[num_readers];

    void* writers_args[] = {(void*) objects, (void*) &num_objects, (void*) &writer_turns};
    void* readers_args[] = {(void*) objects, (void*) &num_objects, (void*) &reader_turns};

    for(int i = 0; i<num_objects; i++){
        objects[i] = allocate_object(initial_data, i);
    }

    for(int i = 0; i<num_writers; i++){
        pthread_create(&writers[i], NULL, writer_thread, writers_args);
    }

    for(int i = 0; i<num_readers; i++){
        pthread_create(&readers[i], NULL, reader_thread, readers_args);
    }

    for(int i = 0; i<num_writers; i++){
        pthread_join(writers[i], NULL);
    }

    for(int i = 0; i<num_readers; i++){
        pthread_join(readers[i], NULL);
    }

    return 0;
}

