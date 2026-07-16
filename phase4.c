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
    int source_record;
    int destination_record;
    int amount;
} WorkerData;

Record records[NUM_RECORDS];

void *database_transaction(void *arg)
{
    WorkerData *data = (WorkerData *)arg;

    int first_lock;
    int second_lock;

    if (data->source_record < data->destination_record) {
        first_lock = data->source_record;
        second_lock = data->destination_record;
    } else {
        first_lock = data->destination_record;
        second_lock = data->source_record;
    }

    printf("Worker %d started with pthread ID %lu\n",
           data->worker_id,
           (unsigned long)pthread_self());

    printf("Worker %d attempting to lock Record %d\n",
           data->worker_id,
           records[first_lock].record_id);

    pthread_mutex_lock(&records[first_lock].mutex);

    printf("Worker %d locked Record %d\n",
           data->worker_id,
           records[first_lock].record_id);

    usleep(100000);

    printf("Worker %d attempting to lock Record %d\n",
           data->worker_id,
           records[second_lock].record_id);

    pthread_mutex_lock(&records[second_lock].mutex);

    printf("Worker %d locked Record %d\n",
           data->worker_id,
           records[second_lock].record_id);

    records[data->source_record].value -= data->amount;
    records[data->destination_record].value += data->amount;

    printf("Worker %d transferred %d from Record %d to Record %d\n",
           data->worker_id,
           data->amount,
           records[data->source_record].record_id,
           records[data->destination_record].record_id);

    pthread_mutex_unlock(&records[second_lock].mutex);
    pthread_mutex_unlock(&records[first_lock].mutex);

    printf("Worker %d completed its transaction\n", data->worker_id);

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
    printf("Phase 4: Deadlock-Free Database Updates\n");
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

    printf("Initial Record 1 value: %d\n", records[0].value);
    printf("Initial Record 2 value: %d\n\n", records[1].value);

    printf("All workers lock records in ascending order.\n\n");

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
    printf("Final total value: %d\n",
           records[0].value + records[1].value);

    if (records[0].value == 110 &&
        records[1].value == 90 &&
        records[0].value + records[1].value == 200) {
        printf("Result: DEADLOCK PREVENTED AND VALUES CORRECT\n");
    } else {
        printf("Result: UNEXPECTED FINAL VALUES\n");
    }

    for (int i = 0; i < NUM_RECORDS; i++) {
        pthread_mutex_destroy(&records[i].mutex);
    }

    return EXIT_SUCCESS;
}
