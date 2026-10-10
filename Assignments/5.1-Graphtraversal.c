/*A network of n locations is represented as a graph. Write a C program that accepts the graph
using an Adjacency Matrix, accepts a starting vertex, performs a graph traversal, displays the visit
order, and ensures that a vertex is not processed repeatedly. Test it with connected and partially
connected graphs.*/


#include <stdio.h>

#define MAX 20

int main() {
    int graph[MAX][MAX], visited[MAX] = {0};
    int queue[MAX];
    int n, start, i, j;
    int front = 0, rear = 0, vertex;

    printf("Enter number of vertices: ");
    scanf("%d", &n);

    printf("Enter adjacency matrix:\n");
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            scanf("%d", &graph[i][j]);
        }
    }

    printf("Enter starting vertex (0 to %d): ", n - 1);
    scanf("%d", &start);

    visited[start] = 1;
    queue[rear++] = start;

    printf("BFS Traversal: ");

    while (front < rear) {
        vertex = queue[front++];
        printf("%d ", vertex);

        for (i = 0; i < n; i++) {
            if (graph[vertex][i] == 1 && !visited[i]) {
                visited[i] = 1;
                queue[rear++] = i;
            }
        }
    }

    printf("\n");

    return 0;
}
