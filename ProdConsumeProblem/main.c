/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>

#define MAX_ITEMS 10 //buffer size, arbitrary number

typedef struct circQueueStruct {
	int start;
	int end;
	int items;
	int data[MAX_ITEMS];
} circQueue;

void initQueue( circQueue *q) {
	//initialize all queue elements to zero
	int i;
	q -> items = 0;
	q -> start = 0;
	q -> end   = 0;
	for (i = 0; i < MAX_ITEMS; ++i) {
		q -> data[i] = 0;
	}
}

int isEmpty( circQueue *q ) {
	//return 1 if queue is empty, else 0
	if ( q->items == 0 ) {
		return 1;
	} else {
		return 0;
	}
}

int isFull( circQueue *q ) {
	//return 1 if queue is full, else 0
	if ( q->items == MAX_ITEMS ) {
		return 1;
	} else {
		return 0;
	}
}

int insertItem( circQueue *q, int item ) {
	if ( isFull(q) ) {
		printf("Queue is full. Can't insert item.");
		return -1;
	} else {
		q->items++; //increment # of items in queue
		q->data[ q->end ] = item; //add to "end" of buffer
		q->end = ( q->end + 1 ) % MAX_ITEMS; //increment & loop around
		return 1;
	}
}

int removeItem( circQueue *q ) {
	//remove item from "start" of buffer
	if ( isEmpty(q) ) {
		printf("Queue is empty. Can't remove item.");
		return -1;
	} else {
		q->items--;
		int result = q->data[ q->start ];
		q->data[ q->start ] = 0; //set start to 0 (removed)
		q->start = ( q->start + 1 ) % MAX_ITEMS; //increment & loop around
		return result;
	}
}

void producer( circQueue *q, sid32 can_consume, sid32 can_produce ) {
	printf("I am producer\n");
	int i;
	for ( i = 1; i < 100; ++i ) { //will test on 100 items
		wait( can_produce );
		insertItem( q, i );
		kprintf( "Producer produced %d\n", i );
		signal( can_consume );
	}
}

void consumer( circQueue *q, sid32 can_consume, sid32 can_produce ) {
	printf("I am consumer\n");
	int removed_item, i;
	for ( i = 1; i < 100; ++i ) { //will test on 100 items
		wait( can_consume );
		removed_item = removeItem( q );
		kprintf( "Consumer consumed %d\n", removed_item );
		signal( can_produce );
	}
}

int main(int argc, char **argv)
{
	circQueue *buffer;
	initQueue( buffer ); //initialize buffer elements to zero (singalling empty slots)
	sid32 can_consume = semcreate( 0 ); //can't consume anything since empty
	sid32 can_produce = semcreate( MAX_ITEMS ); //can produce MAX_ITEMS items until buffer is full
	resume( create(producer, 4096, 1, "producer", 3, buffer, can_consume, can_produce) );
	resume( create(consumer, 4096, 1, "consumer", 3, buffer, can_consume, can_produce) );
	printf("Created processes\n");

	return OK;
}

/* ----- ANALYSIS ----- */
/*  What could go wrong?
		Since each process is sharing access to the buffer, the OS/programmer needs to be sure that
		the producer and consumer processes work together nicely without conflict. This means that
		they cannot access the same element at the same time, the producer can't produce anything if
		the buffer is full, and the consumer can't consume anything if the buffer is empty.
		Producer Problems:
			- Shouldn't produce to full buffer
			- Shouldn't starve the consumer
		Consumer Problems:
			- Shouldn't consume from empty buffer
			- Shouldn't starve the producer

	SOLUTION:
		Here, I will be using semaphores to assure that the producer and consumer play nicely. I have
		two semaphores: can_consume, which indicates if the consumer is safe to consume items from the
		buffer, and can_produce, which indicates if the producer is safe to produce items to the buffer.
		The inital value of can_consume is 0 because the buffer is empty at the start and is therefore 
		unsafe to consume items from. The initial value of can_produce is MAX_ITEMS, because the producer
		can produce MAX_ITEMS until the buffer is full. I then create each producer and consumer process
		with the same priority. Each process waits until its respective semaphore is signalled, then
		proceeds with consuming/producing. After that, it signals the opposite semaphore. This assures
		that whenever the producer produces, it says "I created a new item which can now be consumed."
		and vice versa for the consumer.