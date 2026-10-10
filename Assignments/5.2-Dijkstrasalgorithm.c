/*A transportation network contains cities connected by roads with different costs. Write a C
program implementing Dijkstra’s Shortest Path Algorithm that accepts the number of vertices,
weighted adjacency matrix and source vertex, computes the minimum distance from the source to
every other vertex, and displays each destination with its shortest distance. Test it using at least
five vertices.*/


#include <stdio.h>

#define MAX 20
#define INF 99999

int main() {
    int cost[MAX][MAX], dist[MAX], visited[MAX];
    int n, source, i, j, count;
    int min, u;

    printf("Enter number of vertices: ");
    scanf("%d", &n);

    printf("Enter weighted adjacency matrix (0 for no edge):\n");

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            scanf("%d", &cost[i][j]);

            if (i != j && cost[i][j] == 0)
                cost[i][j] = INF;
        }
    }

    printf("Enter source vertex (0 to %d): ", n - 1);
    scanf("%d", &source);

    for (i = 0; i < n; i++) {
        dist[i] = cost[source][i];
        visited[i] = 0;
    }

    dist[source] = 0;
    visited[source] = 1;

    for (count = 1; count < n; count++) {
        min = INF;
        u = -1;

        for (i = 0; i < n; i++) {
            if (!visited[i] && dist[i] < min) {
                min = dist[i];
                u = i;
            }
        }

        if (u == -1)
            break;

        visited[u] = 1;

        for (j = 0; j < n; j++) {
            if (!visited[j] &&
                cost[u][j] != INF &&
                dist[u] != INF &&
                dist[u] + cost[u][j] < dist[j]) {

                dist[j] = dist[u] + cost[u][j];
            }
        }
    }

    printf("\nShortest distances from vertex %d:\n", source);

    for (i = 0; i < n; i++) {
        if (dist[i] == INF)
            printf("To vertex %d = Unreachable\n", i);
        else
            printf("To vertex %d = %d\n", i, dist[i]);
    }

    return 0;
}
