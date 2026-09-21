/*Develop a C program for a Doubly Linked List representing a sequence of web pages visited by a
user. The program should insert a new page, move forward and backward, delete a specified
page, and display the pages from first-to-last and last-to-first while handling beginning and end
conditions correctly.*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node {
    char page[100];
    struct Node *prev;
    struct Node *next;
};

struct Node *head = NULL;
struct Node *tail = NULL;
struct Node *current = NULL;

/* Insert a new page */
void insertPage(char page[]) {
    struct Node *newNode;

    newNode = (struct Node *)malloc(sizeof(struct Node));

    strcpy(newNode->page, page);
    newNode->prev = NULL;
    newNode->next = NULL;

    if (head == NULL) {
        head = tail = current = newNode;
    } else {
        newNode->prev = tail;
        tail->next = newNode;
        tail = newNode;
    }

    printf("Page '%s' inserted.\n", page);
}

/* Move forward */
void moveForward() {
    if (current == NULL) {
        printf("No pages available.\n");
    }
    else if (current->next == NULL) {
        printf("Already at the last page. Cannot move forward.\n");
    }
    else {
        current = current->next;
        printf("Moved forward to: %s\n", current->page);
    }
}

/* Move backward */
void moveBackward() {
    if (current == NULL) {
        printf("No pages available.\n");
    }
    else if (current->prev == NULL) {
        printf("Already at the first page. Cannot move backward.\n");
    }
    else {
        current = current->prev;
        printf("Moved backward to: %s\n", current->page);
    }
}

/* Delete a specified page */
void deletePage(char page[]) {
    struct Node *temp = head;

    while (temp != NULL && strcmp(temp->page, page) != 0) {
        temp = temp->next;
    }

    if (temp == NULL) {
        printf("Page '%s' not found.\n", page);
        return;
    }

    /* If deleting the first page */
    if (temp == head) {
        head = temp->next;

        if (head != NULL)
            head->prev = NULL;
    }
    else {
        temp->prev->next = temp->next;
    }

    /* If deleting the last page */
    if (temp == tail) {
        tail = temp->prev;

        if (tail != NULL)
            tail->next = NULL;
    }
    else {
        temp->next->prev = temp->prev;
    }

    /* Update current page */
    if (temp == current) {
        if (temp->next != NULL)
            current = temp->next;
        else
            current = temp->prev;
    }

    free(temp);

    printf("Page '%s' deleted.\n", page);
}

/* Display first to last */
void displayForward() {
    struct Node *temp = head;

    if (head == NULL) {
        printf("No pages available.\n");
        return;
    }

    printf("Pages (First to Last):\n");

    while (temp != NULL) {
        printf("%s", temp->page);

        if (temp->next != NULL)
            printf(" <-> ");

        temp = temp->next;
    }

    printf("\n");
}

/* Display last to first */
void displayBackward() {
    struct Node *temp = tail;

    if (tail == NULL) {
        printf("No pages available.\n");
        return;
    }

    printf("Pages (Last to First):\n");

    while (temp != NULL) {
        printf("%s", temp->page);

        if (temp->prev != NULL)
            printf(" <-> ");

        temp = temp->prev;
    }

    printf("\n");
}

/* Main function */
int main() {
    int choice;
    char page[100];

    while (1) {
        printf("\n--- Web Page History ---\n");
        printf("1. Insert New Page\n");
        printf("2. Move Forward\n");
        printf("3. Move Backward\n");
        printf("4. Delete Page\n");
        printf("5. Display First to Last\n");
        printf("6. Display Last to First\n");
        printf("7. Exit\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {

            case 1:
                printf("Enter page name: ");
                scanf("%s", page);

                insertPage(page);
                displayForward();
                break;

            case 2:
                moveForward();
                break;

            case 3:
                moveBackward();
                break;

            case 4:
                printf("Enter page to delete: ");
                scanf("%s", page);

                deletePage(page);
                displayForward();
                break;

            case 5:
                displayForward();
                break;

            case 6:
                displayBackward();
                break;

            case 7:
                printf("Program terminated.\n");
                return 0;

            default:
                printf("Invalid choice.\n");
        }
    }

    return 0;
}