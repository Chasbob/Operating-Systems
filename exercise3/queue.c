#include "queue.h"

// A utility function to create a new linked list node. 
struct QNode* newNode(char* k) 
{ 
	struct QNode* temp = (struct QNode*)kmalloc(sizeof(struct QNode), GFP_KERNEL); 
	temp->key = k; 
	temp->next = NULL; 
	return temp; 
} 

// A utility function to create an empty queue 
struct Queue* createQueue(void) 
{ 
	struct Queue* q;
   q = (struct Queue*)kmalloc(sizeof(struct Queue), GFP_KERNEL);
	q->front = q->rear = NULL; 
	q->size=0;
	return q; 
} 

// The function to add a key k to q 
 void enQueue(struct Queue* q, char* k) 
{ 
	// Create a new LL node 
	struct QNode* temp = newNode(k); 

	// If queue is empty, then new node is front and rear both 
	if (q->rear == NULL) { 
		q->front = q->rear = temp; 
		q->size = q->size+1;
		return; 
	} 

	// Add the new node at the end of queue and change rear 
	q->rear->next = temp; 
	q->rear = temp; 
} 

// Function to remove a key from given queue q 
struct QNode* deQueue(struct Queue* q) 
{ 
	struct QNode* temp;
	// If queue is empty, return NULL. 
	if (q->front == NULL) 
		return NULL; 

	// Store previous front and move front one node ahead 
   temp =  q->front; 
	kfree(temp); 

	q->front = q->front->next; 

	// If front becomes NULL, then change rear also as NULL 
	if (q->front == NULL) 
		q->rear = NULL; 
	q->size = q->size  - 1;
	return temp; 
} 

 char* peek(struct Queue* q){
	return q->front->key;
}

 char* peekTail(struct Queue* q){
	return q->rear->key;
}