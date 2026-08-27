/*A company stores employee IDs in ascending order. Write a C program that accepts n employee
IDs, searches for a required ID using Binary Search, displays its position when found, reports
when it is absent, and counts the number of comparisons. Test the program for both successful
and unsuccessful searches.*/

#include <stdio.h>

int binarySearch(int arr[], int n, int key, int *comparisons) {
    int low = 0, high = n - 1, mid;
    *comparisons = 0;

    while (low <= high) {
        mid = low + (high - low) / 2;
        (*comparisons)++;

        if (arr[mid] == key) {
            return mid;
        } 
        else if (arr[mid] < key) {
            low = mid + 1;
        } 
        else {
            high = mid - 1;
        }
    }
    return -1;
}

int main() {
    int n, key, pos, comparisons;

    printf("Enter number of employee IDs: ");
    scanf("%d", &n);

    int ids[n];
    printf("Enter %d employee IDs in ascending order:\n", n);
    for (int i = 0; i < n; i++) {
        scanf("%d", &ids[i]);
    }

    printf("Enter the employee ID to search: ");
    scanf("%d", &key);

    pos = binarySearch(ids, n, key, &comparisons);

    if (pos != -1) {
        printf("\nEmployee ID %d found at position %d (index %d).\n", key, pos + 1, pos);
    } else {
        printf("\nEmployee ID %d not found in the records.\n", key);
    }

    printf("Number of comparisons made: %d\n", comparisons);

    return 0;
}