
/*A service centre uses a fixed-size request buffer in which released positions must be reused.
Write a C program to implement a Circular Queue using an array with insertion, deletion, display,
overflow and underflow operations. Demonstrate that positions freed after deletion can be reused
for new requests.*/

#include <stdio.h>
#include <stdbool.h>

#define MAX_SIZE 5  
typedef struct {
    int items[MAX_SIZE];
    int front;
    int rear;
} CircularQueue;

// Initialize the circular queue
void initQueue(CircularQueue *q) {
    q->front = -1;
    q->rear = -1;
}

bool isEmpty(CircularQueue *q) {
    return (q->front == -1);
}

bool isFull(CircularQueue *q) {
    return ((q->rear + 1) % MAX_SIZE == q->front);
}

void enqueue(CircularQueue *q, int requestId) {
    if (isFull(q)) {
        printf("[OVERFLOW] Buffer is full! Cannot add Request #%d.\n", requestId);
        return;
    }

    if (isEmpty(q)) {
        q->front = 0;
        q->rear = 0;
    } else {
        q->rear = (q->rear + 1) % MAX_SIZE;
    }

    q->items[q->rear] = requestId;
    printf("[ENQUEUE] Request #%d added at array index [%d]. (Front: %d, Rear: %d)\n",
           requestId, q->rear, q->front, q->rear);
}

int dequeue(CircularQueue *q) {
    if (isEmpty(q)) {
        printf("[UNDERFLOW] Buffer is empty! No requests to service.\n");
        return -1;
    }

    int removed = q->items[q->front];
    int removedIndex = q->front;

    if (q->front == q->rear) {

        q->front = -1;
        q->rear = -1;
    } else {
        q->front = (q->front + 1) % MAX_SIZE;
    }

    printf("[DEQUEUE] Serviced Request #%d from array index [%d]. (Front: %d, Rear: %d)\n",
           removed, removedIndex, q->front, q->rear);
    return removed;
}

void display(CircularQueue *q) {
    if (isEmpty(q)) {
        printf("[DISPLAY] Queue is empty.\n\n");
        return;
    }

    printf("[DISPLAY] Active Requests (Front -> Rear): ");
    int i = q->front;
    while (true) {
        printf("[Index %d: Req #%d] ", i, q->items[i]);
        if (i == q->rear) break;
        i = (i + 1) % MAX_SIZE;
    }
    printf("\n\n");
}

int main() {
    CircularQueue q;
    initQueue(&q);

    printf("===============================================================\n");
    printf("  SERVICE CENTRE REQUEST BUFFER DEMO (BUFFER CAPACITY: %d)\n", MAX_SIZE);
    printf("===============================================================\n\n");

    printf("--- Test 1: Underflow Condition ---\n");
    dequeue(&q);
    display(&q);

    printf("--- Test 2: Filling the Buffer ---\n");
    enqueue(&q, 101);
    enqueue(&q, 102);
    enqueue(&q, 103);
    enqueue(&q, 104);
    enqueue(&q, 105);
    display(&q);

    printf("--- Test 3: Overflow Condition ---\n");
    enqueue(&q, 106);
    printf("\n");

    printf("--- Test 4: Servicing Requests (Freeing slots 0 and 1) ---\n");
    dequeue(&q); 
    dequeue(&q);
    display(&q);

    printf("--- Test 5: Reusing Released Positions ---\n");
    printf("Adding new requests (107, 108) which will wrap around to index 0 and 1:\n");
    enqueue(&q, 107);
    enqueue(&q, 108);
    display(&q);

    printf("--- Test 6: Buffer Full Again After Wraparound ---\n");
    enqueue(&q, 109);
    printf("\n");

    return 0;
}