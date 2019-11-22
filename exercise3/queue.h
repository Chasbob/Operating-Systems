struct QNode { 
	char* key; 
	struct QNode* next; 
}; 
struct Queue { 
	struct QNode *front, *rear; 
	int size;
}; 
struct QNode* newNode(char* k) ;

struct Queue* createQueue(void) ;

void enQueue(struct Queue* q, char* k) ;

struct QNode* deQueue(struct Queue* q) ;

char* peek(struct Queue* q);

char* peekTail(struct Queue* q);