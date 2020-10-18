#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct{
    int data;
    int writers_count;
    int num_writers;
    int readers_count;
    bool* writers_record;

    pthread_mutex_t writer_mutex;
    pthread_mutex_t reader_mutex;
    pthread_cond_t critic_cond;
}concurrent_object;

concurrent_object allocate_object(int initial_data, int num_writers){
    concurrent_object object;
    object.data = initial_data;
    object.num_writers = num_writers;
    object.readers_count = 0;
    object.writers_count = 0;
    object.writers_record = (bool*) malloc(num_writers * sizeof(bool));

    for(int i = 0; i<num_writers; i++){
        object.writers_record[i] = false;
    }

    pthread_mutex_init(&object.writer_mutex, NULL);
    pthread_mutex_init(&object.reader_mutex, NULL);
    pthread_cond_init(&object.critic_cond, NULL);
    return object;
}

void deallocate_object(concurrent_object* object){
    free(object->writers_record);
}

void read_from_object(concurrent_object* object){
    pthread_mutex_lock(&object->reader_mutex);
    object->readers_count++;
    if(object->readers_count == 1){
        pthread_mutex_lock(&object->writer_mutex);
    }
    pthread_mutex_unlock(&object->reader_mutex);
    printf("reader of id: %ld read from object: %d\n", pthread_self(), object->data);
    pthread_mutex_lock(&object->reader_mutex);
    object->readers_count--;
    if(object->readers_count == 0){
        pthread_mutex_unlock(&object->writer_mutex);
    }
    pthread_mutex_unlock(&object->reader_mutex);
}

void write_to_object(concurrent_object* object, int writer_idx){
    pthread_mutex_lock(&object->writer_mutex);
    while(object->writers_count == object->num_writers){
        pthread_cond_wait(&object->critic_cond, &object->writer_mutex);
    }

    if(!object->writers_record[writer_idx]){
        object->writers_count++;
        object->writers_record[writer_idx] = true;
    }

    if(object->writers_count == object->num_writers){
        pthread_cond_broadcast(&object->critic_cond);
    }

    object->data = rand();
    printf("writer of id: %ld wrote to object: %d\n", pthread_self(), object->data);
    pthread_mutex_unlock(&object->writer_mutex);
}

static void critic_clean_up(void* args){
    concurrent_object* object = (concurrent_object*) args;
    if(object->writers_count == object->num_writers){
        printf("Critic saved data: %d\n", object->data);
    }
}

void critic_round(concurrent_object* object){
    pthread_mutex_lock(&object->writer_mutex);
    while(object->writers_count != object->num_writers){
        pthread_cond_wait(&object->critic_cond, &object->writer_mutex);
    }

    object->writers_count = 0;
    for(int i = 0; i<object->num_writers; i++){
        object->writers_record[i] = false;
    }

    printf("Critic saved data: %d\n", object->data);

    pthread_cond_broadcast(&object->critic_cond);
    pthread_mutex_unlock(&object->writer_mutex);
}

void* writer_thread(void* args){
    void** args_arr = (void**) args;
    concurrent_object* object = (concurrent_object*) args_arr[0];
    int* writer_runs = (int*) args_arr[1];
    int* writer_idx = (int*) args_arr[2];

    for(int i = 0; i<*writer_runs; i++){
        write_to_object(object, *writer_idx);
    }
}

void* reader_thread(void* args){
    void** args_arr = (void**) args;
    concurrent_object* object = (concurrent_object*) args_arr[0];
    int* reader_runs = (int*) args_arr[1];

    for(int i = 0; i<*reader_runs; i++){
        read_from_object(object);
    }
}

void* critic_thread(void* args){
    concurrent_object* object = (concurrent_object*) args;
    pthread_cleanup_push(critic_clean_up, (void*) object);
    while(true){
        critic_round(object);
    }
    pthread_cleanup_pop(0);
}

int main(){
    int num_writers = 100;
    int num_readers = 200;

    int writer_runs = 11;
    int reader_runs = 14;

    int initial_data = 0;

    pthread_t writers[num_writers];
    pthread_t readers[num_readers];
    pthread_t critic;

    concurrent_object object = allocate_object(initial_data, num_writers);

    void* reader_args[] = {(void*) &object, (void*) &reader_runs};
    int writers_idx[num_writers];

    void* writers_args[num_writers][3];

    for(int i = 0; i<num_writers; i++){
        writers_idx[i] = i;
        writers_args[i][0] = (void*) &object;
        writers_args[i][1] = (void*) &writer_runs;
        writers_args[i][2] = (void*) &writers_idx[i];
    }

    for(int i = 0; i<num_writers; i++){
        pthread_create(&writers[i], NULL, writer_thread, (void*) writers_args[i]);
    }

    for(int i = 0; i<num_readers; i++){
        pthread_create(&readers[i], NULL, reader_thread, (void*) reader_args);
    }

    pthread_create(&critic, NULL, critic_round, (void*) &object);

    for(int i = 0; i<num_readers; i++){
        pthread_join(readers[i], NULL);
    }

    for(int i = 0; i<num_writers; i++){
        pthread_join(writers[i], NULL);
    }
    pthread_cancel(critic);
    pthread_join(critic, NULL);
    deallocate_object(&object);

    return 0;
}




