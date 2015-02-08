/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>
#include <stdlib.h>


void philosopher( char name, sid32 fork_low, sid32 fork_high, sid32 print ) { //takes in name, low fork and high fork
	int i = 0;
	for ( i = 0; i < 150; i++ ) { //run through 150 iterations of eating/waiting
		sleep( rand() * 1000000 ); //"thinks" for a random time before deciding to eat again
		wait( fork_low ); //waits on lower # fork
		wait( print );
		kprintf( "Phil %c has grabbed low fork.\n", name );
		signal( print );
		wait( fork_high ); //waits of higher # fork, already has other fork in hand
		wait( print );
		kprintf( "Phil %c has grabbed high fork and is ready to eat.\n", name );
		signal( print );
		sleep( rand() * 1000000 ); //"eats" for a random time
		wait( print );
		kprintf( "Phil %c has finished eating and will now signal.\n", name );
		signal( print );
		signal( fork_high ); //signals high fork first, creates a mutex of the high fork with line 13
		signal( fork_low ); //signals low fork next, creates a mutex of the low fork with line 11
		wait( print );
		kprintf( "Phil %c is now thinking.\n", name );
		kprintf( "On iteration %d.\n", i );
		signal( print );
		//This implementation is gridlock free because of the mutual exclusion created with the low and high forks.
		//Each fork can only be used by one philosopher at a time, and can only "eat" if they have both forks.
		//After they are done eating, they signal both forks so that they are made available to the other philosophers.
	}
	kprintf( "Phil %c is done with 150 iterations.", name );
}



int main(int argc, char **argv)
{
	sched_cntl( DEFER_START );
	srand( 5 ); // initialize random function with arbitrary seed
	sid32 fork1 = semcreate( 1 ); //fork 1 semaphore
	sid32 fork2 = semcreate( 1 ); //fork 2 semaphore
	sid32 fork3 = semcreate( 1 ); //fork 3 semaphore
	sid32 fork4 = semcreate( 1 ); //fork 4 semaphore
	sid32 fork5 = semcreate( 1 ); //fork 5 semaphore
	sid32 print = semcreate( 1 );
	//each philosopher has a left fork and a right fork, they will always grab lower # fork first
	resume( create(philosopher, 4096, 50, "philosopherA", 4, 'A', fork1, fork2, print ) ); //philosopher A has forks 1 & 2
	resume( create(philosopher, 4096, 50, "philosopherB", 4, 'B', fork2, fork3, print ) ); //philosopher B has forks 2 & 3
	resume( create(philosopher, 4096, 50, "philosopherC", 4, 'C', fork3, fork4, print ) ); //philosopher C has forks 3 & 4
	resume( create(philosopher, 4096, 50, "philosopherD", 4, 'D', fork4, fork5, print ) ); //philosopher D has forks 4 & 5
	resume( create(philosopher, 4096, 50, "philosopherE", 4, 'E', fork1, fork5, print ) ); //philosopher E has forks 1 & 5
	printf("Created philosophers\n");
	sched_cntl( DEFER_STOP );

	return OK;
}

