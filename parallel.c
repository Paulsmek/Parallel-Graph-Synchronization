#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include "os_graph.h"
#include "os_threadpool.h"
#include "log/log.h"
#include "utils.h"

#define NUM_THREADS		4

static int sum;
static os_graph_t *graph;
static os_threadpool_t *tp;
/* TODO: Define graph synchronization mechanisms. */
pthread_mutex_t lock;

/* TODO: Define graph task argument. */

void process_node(unsigned int idx)
{
	os_node_t *node = graph->nodes[idx];

	pthread_mutex_lock(&lock);

	if (graph->visited[idx] != DONE) {
		graph->visited[idx] = DONE;
		sum += node->info;
	}
	pthread_mutex_unlock(&lock);

	for (unsigned int i = 0; i < node->num_neighbours; i++) {
		if (graph->visited[node->neighbours[i]] != DONE) {
			os_task_t *task = create_task((void (*)(void *))process_node, (void *)(node->neighbours[i]), NULL);

			enqueue_task(tp, task);
		}
	}
}


int main(int argc, char *argv[])
{
	FILE *input_file;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s input_file\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	input_file = fopen(argv[1], "r");

	DIE(input_file == NULL, "fopen");

	graph = create_graph_from_file(input_file);

	/* TODO: Initialize graph synchronization mechanisms. */
	tp = create_threadpool(NUM_THREADS);

	pthread_mutex_init(&lock, NULL);

	for (unsigned int i = 0; i < graph->num_nodes; i++) {
		if (graph->visited[i] == NOT_VISITED && (graph->nodes[i]->num_neighbours > 0 || i == 0)) {
			os_task_t *task = create_task((void (*)(void *))process_node, (void *)(i), NULL);

			enqueue_task(tp, task);
		}
	}

	wait_for_completion(tp);
	pthread_mutex_destroy(&lock);
	destroy_threadpool(tp);

	printf("%d", sum);

	return 0;
}
