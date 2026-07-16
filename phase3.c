#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_RECORDS 2
#define NUM_WORKERS 2

typedef struct {
    int record_id;
    int value;
    pthread_mutex_t mutex;
} Record;

typedef struct {
    int worker_id;
    int first_record;
    int second_record;
    int amount;
} WorkerData;

Record records[NUM_RECORDS];
pthread_barrier_t barrier;

void *database_transaction(void *arg)
{
    WorkerData *data = (WorkerData *)arg;

    printf("Worker %d started with pthread ID %lu\n",
           data->worker_id,
           (unsigned long)pthread_self());

    printf("Worker %d attempting to lock Record %d\n",
           data->worker_id,
           records[data->first_record].record_id);

    pthread_mutex_lock(&records[data->first_record].mutex);

    printf("Worker %d locked Record %d\n",
           data->worker_id,
           records[data->first_record].record_id);

    pthread_barrier_wait(&barrier);

    sleep(1);

    printf("Worker %d attempting to lock Record %d\n",
           data->worker_id,
           records[data->second_record].record_id);

    pthread_mutex_lock(&records[data->second_record].mutex);

    records[data->first_record].value -= data->amount;
    records[data->second_record].value += data->amount;

    printf("Worker %d completed its transaction\n", data->worker_id);

    pthread_mutex_unlock(&records[data->second_record].mutex);
    pthread_mutex_unlock(&records[data->first_record].mutex);

    return NULL;
}

int main(void)
{
    pthread_t workers[NUM_WORKERS];

    WorkerData worker_data[NUM_WORKERS] = {
        {1, 0, 1, 10},
        {2, 1, 0, 20}
    };

    setvbuf(stdout, NULL, _IONBF, 0);

    printf("========================================\n");
    printf("Phase 3: Intentional Database Deadlock\n");
    printf("========================================\n\n");

    for (int i = 0; i < NUM_RECORDS; i++) {
        records[i].record_id = i + 1;
        records[i].value = 100;

        if (pthread_mutex_init(&records[i].mutex, NULL) != 0) {
            fprintf(stderr,
                    "Failed to initialize mutex for Record %d\n",
                    i + 1);
            return EXIT_FAILURE;
        }
    }

    if (pthread_barrier_init(&barrier, NULL, NUM_WORKERS) != 0) {
        fprintf(stderr, "Failed to initialize barrier\n");
        return EXIT_FAILURE;
    }

    printf("Initial Record 1 value: %d\n", records[0].value);
    printf("Initial Record 2 value: %d\n\n", records[1].value);

    printf("Worker 1 will lock Record 1 and then Record 2\n");
    printf("Worker 2 will lock Record 2 and then Record 1\n\n");

    for (int i = 0; i < NUM_WORKERS; i++) {
        int result = pthread_create(
            &workers[i],
            NULL,
            database_transaction,
            &worker_data[i]
        );

        if (result != 0) {
            fprintf(stderr, "Failed to create Worker %d\n", i + 1);
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < NUM_WORKERS; i++) {
        int result = pthread_join(workers[i], NULL);

        if (result != 0) {
            fprintf(stderr, "Failed to join Worker %d\n", i + 1);
            return EXIT_FAILURE;
        }
    }

    printf("\nFinal Record 1 value: %d\n", records[0].value);
    printf("Final Record 2 value: %d\n", records[1].value);

    pthread_barrier_destroy(&barrier);

    for (int i = 0; i < NUM_RECORDS; i++) {
        pthread_mutex_destroy(&records[i].mutex);
    }

    return EXIT_SUCCESS;
}
