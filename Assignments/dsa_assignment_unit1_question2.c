 /*A teacher wants to arrange student marks in ascending order and also measure how much
rearrangement is necessary. Write a C program using Insertion Sort that accepts n marks, displays
the array after every pass, counts the total number of element shifts, and displays the final sorted
list and shift count.*/
 
 #include <stdio.h>

int main() {
    int n, i, j, key;
    int shifts = 0;

    printf("enter no.of students");
    scanf("%d", &n);

    int marks[n];

    printf("enter student marks:");
    for (i = 0; i < n; i++) {
        scanf("%d", &marks[i]);
    }

    
    for (i = 1; i < n; i++) {
        key = marks[i];
        j = i - 1;

        
        while (j >= 0 && marks[j] > key) {
            marks[j + 1] = marks[j];
            j--;
            shifts++;
        }

        
        marks[j + 1] = key;

        
        printf("Pass %d: ", i);
        for (j = 0; j < n; j++) {
            printf("%d ", marks[j]);
        }
        printf("\n");
    }

    
    printf("Sorted array: ");
    for (i = 0; i < n; i++) {
        printf("%d ", marks[i]);
    }

    printf("\nTotal shifts: %d\n", shifts);

    return 0;
}