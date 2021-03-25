/*
 * Part1.cpp
 *
 *  Created on: Mar 19, 2021
 *      Author: Sam's Desktop
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>

// Macros for easy reading within code
#define ZERO_OPTIONS -1
#define DEFAULT_NUMBER_OF_PRODUCERS 1
#define DEFAULT_BUFFER_LENGTH 10
#define DEFAULT_INSERTIONS 100

// Global variables shared among all threads
char *buffer;
sem_t empty;
sem_t full;
sem_t mutex;
int items;
int count = 0;
int total = 0;

void* producer(void *args) {
	// Get the thread ID for producer
	int threadID = *(int*) args;
	while (true) {
		sem_wait(&empty);
		sem_wait(&mutex);

		// Check to see if number of insertions has exceed expected
		// insertions
		if (items <= total)
			break;

		// Insert X into the first available slot in the buffer
		buffer[count] = 'X';
		printf("p:<%d>, item: %c, at %d\n", threadID, 'X', count);
		count++;
		total++;
		sem_post(&mutex);
		sem_post(&full);
	}
	sem_post(&mutex);
	sem_post(&full);
}

void* consumer(void *args) {
	// Get the thread ID for consumer
	int threadID = *(int*) args;
	while (true) {
		sem_wait(&full);
		sem_wait(&mutex);

		// Check to see if the consumers have consumed all
		// insertions
		if (count == 0 && total == items)
			exit(EXIT_SUCCESS);

		// Remove X from the last used slot in the buffer
		printf("c:<%d>, item: %c, at %d\n", threadID, buffer[count - 1],
				count - 1);
		count--;
		sem_post(&mutex);
		sem_post(&empty);
	}
}

int main(int argc, char *argv[]) {
	// Assign variables to their default value
	int bufferLength = DEFAULT_BUFFER_LENGTH;
	int numberOfProducers = DEFAULT_NUMBER_OF_PRODUCERS;
	int numberOfConsumers = DEFAULT_NUMBER_OF_PRODUCERS;
	int numberOfInsertions = DEFAULT_INSERTIONS;
	int option;
	// Reassign variables if their proper switch is given
	while ((option = getopt(argc, argv, "b:p:c:i:")) != ZERO_OPTIONS) {

		if (option == ZERO_OPTIONS)
			break;

		switch (option) {
		case 'b':
			bufferLength = atoi(optarg);
			break;
		case 'p':
			numberOfProducers = atoi(optarg);
			break;
		case 'c':
			numberOfConsumers = atoi(optarg);
			break;
		case 'i':
			numberOfInsertions = atoi(optarg);
			break;
		default:
			exit(EXIT_FAILURE);

		}
	}

	items = numberOfInsertions;
	// Create the buffer
	buffer = (char*) malloc(bufferLength * sizeof(char));
	// Initialize the semaphores
	sem_init(&empty, 0, bufferLength);
	sem_init(&full, 0, 0);
	sem_init(&mutex, 0, 1);
	// Create a thread variable
	pthread_t thread;

	if (numberOfProducers > 0) {
		// create producer threads
		for (int i = 0; i < numberOfProducers; i++) {
			pthread_create(&thread, NULL, &producer, &i);

		}
	}

	if (numberOfConsumers > 0) {
		// create consumer threads
		for (int i = 0; i < numberOfConsumers; i++) {
			pthread_create(&thread, NULL, &consumer, &i);
		}
	}

	if (numberOfProducers > 0) {
		for (int i = 0; i < numberOfProducers; i++)
			pthread_join(thread, NULL);
	}

	if (numberOfConsumers > 0) {
		for (int i = 0; i < numberOfConsumers; i++)
			pthread_join(thread, NULL);
	}
	// Destroy the semaphores
	sem_destroy(&empty);
	sem_destroy(&full);
	sem_destroy(&mutex);

	return EXIT_SUCCESS;
}

