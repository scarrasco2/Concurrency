/*
 * Part2.cpp
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

// Definition for Monitor type
typedef struct Monitor {
	int numberOfSignals = 0;
	sem_t signal;
} Monitor;

// Definition for Condition Variable type
typedef struct Condition {
	int waitingThreads = 0;
	sem_t queue;
} Conditon;


// Global variables shared among all threads
Monitor monitor;
Condition full;
Condition empty;
sem_t mutex;
char *buffer;
int items;
int count = 0;
int total = 0;
int buffer_length;
time_t seed;

int countCV(Condition &conditionVariable) {
	return conditionVariable.waitingThreads;
}

void waitCV(Condition &conditionVariable) {
	conditionVariable.waitingThreads++;
	if (monitor.numberOfSignals > 0) {
		sem_post(&monitor.signal);
	} else {
		sem_post(&mutex);
	}
	sem_wait(&conditionVariable.queue);
	conditionVariable.waitingThreads--;
}

void signalCV(Condition &conditionVariable) {
	if (conditionVariable.waitingThreads > 0) {
		monitor.numberOfSignals++;
		sem_post(&conditionVariable.queue);
		sem_wait(&monitor.signal);
		monitor.numberOfSignals--;
	}

}

void insert(char item, int threadID) {
	// Lock the critical section
	sem_wait(&mutex);

	// Check to see if the producers have
	// produced all items
	if (total == items) {
		sem_post(&mutex);
		pthread_exit(EXIT_SUCCESS);
	}

	// Once the buffer is full
	// inform the full condition variable
	while (count == buffer_length)
		waitCV(full);

	// Put the random character into the buffer
	if (total < items && count < buffer_length) {
		buffer[count] = item;
		total++;
		printf("p:<%d>, item: %c, at %d\n", threadID, item, count++);
	}

	signalCV(empty);

	if (monitor.numberOfSignals > 0)
		sem_post(&monitor.signal);
	else
		sem_post(&mutex);

}

void remove(int threadID) {

	// Lock the critical section
	sem_wait(&mutex);

	// Check to see if the consumers have
	// consumed all the slots in the buffer
	if (total == items && count == 0)
		exit(EXIT_SUCCESS);

	// Once the buffer is empty
	// inform the empty condition variable
	while (count == 0)
		waitCV(empty);

	// Remove the random character from the buffer
	if (total <= items && count > 0) {
		char item = buffer[count - 1];
		printf("c:<%d>, item: %c, at %d\n", threadID, item, --count);
	}

	signalCV(full);

	if (monitor.numberOfSignals > 0)
		sem_post(&monitor.signal);
	else
		sem_post(&mutex);
}

void* producer(void *args) {
	// Get the thread ID for producer
	int threadID = *(int*) args;
	const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			               "abcdefghijklmnopqrstuvwxyz";
	while (true) {
		// Generate random character to be inserted in
		// the buffer
		char letter = letters[rand() % 52];
		insert(letter, threadID);
	}
	return EXIT_SUCCESS;
}

void* consumer(void *args) {
	// Get the thread ID for consumer
	int threadID = *(int*) args;
	while (true)
		remove(threadID);

	return EXIT_SUCCESS;
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

	// Assign the global variables
	buffer_length = bufferLength;
	items = numberOfInsertions;
	sem_init(&(full.queue), 0, 0);
	sem_init(&(empty.queue), 0, 0);
	sem_init(&mutex, 0, 1);
	sem_init(&(monitor.signal), 0, 0);

	// Create the buffer
	buffer = (char*) malloc(bufferLength * sizeof(char));

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

	return EXIT_SUCCESS;
}
