/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>

#define MAX_ITEMS 10

typedef struct circQueueStruct {
	int start;
	int end;
	int items;
	int data[MAX_ITEMS];
} circQueue;

void initQueue( circQueue *q) {
	//initialize all queue elements to zero
	q -> items = 0;
	q -> start = 0;
	q -> end   = 0;
	for (int i = 0; i < MAX_ITEMS; ++i) {
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

void producer( circQueue *q, int can_consume, int can_produce ) {
	for ( int i = 1; i < 100; ++i ) {
		wait( can_produce );
		insertItem( q, i );
		kprintf( "Producer produced %d\n", i );
		signal( can_consume );
	}
}

void consumer( circQueue *q, int can_consume, int can_produce ) {
	int removed_item;
	for ( int i = 1; i < 100; ++i ) {
		wait( can_consume );
		removed_item = removeItem( q );
		kprintf( "Consumer consumed %d\n", removed_item );
		signal( can_produce );
	}
}

int main(int argc, char **argv)
{
	circQueue buffer;
	initQueue( &buffer );
	int can_consume = screate(0);
	int can_produce = screate(1);
	resume( create(producer, 4096, 2, "producer", 3, buffer, can_consume, can_produce) );
	resume( create(consumer, 4096, 1, "consumer", 3, buffer, can_consume, can_produce) );
	while(TRUE) {
		// Do nothing
	}

	/*
	uint32 retval;

	resume(create(shell, 8192, 50, "shell", 1, CONSOLE));

	// Wait for shell to exit and recreate it

	recvclr();
	while (TRUE) {
		retval = receive();
		kprintf("\n\n\rMain process recreating shell\n\n\r");
		resume(create(shell, 4096, 1, "shell", 1, CONSOLE));
	}
	while (1);
	*/
	return OK;
}
