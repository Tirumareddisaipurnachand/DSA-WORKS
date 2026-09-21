/*A department maintains student roll numbers dynamically. Write a C program using a Singly
Linked List to create the list, insert at the beginning and end, search for a specified roll number,
delete a specified roll number, and display the updated list after each operation. Handle the case
when a requested roll number is not available*/



#include <stdio.h>
#include <stdlib.h>

struct Node {
    int roll;
    struct Node *next;
};

struct Node *head = NULL;

/* Insert at beginning */
void insertBeginning(int roll) {
    struct Node *newNode;

    newNode = (struct Node *)malloc(sizeof(struct Node));

    newNode->roll = roll;
    newNode->next = head;
    head = newNode;

    printf("Roll number %d inserted at beginning.\n", roll);
}

/* Insert at end */
void insertEnd(int roll) {
    struct Node *newNode, *temp;

    newNode = (struct Node *)malloc(sizeof(struct Node));

    newNode->roll = roll;
    newNode->next = NULL;

    if (head == NULL) {
        head = newNode;
    } else {
        temp = head;

        while (temp->next != NULL) {
            temp = temp->next;
        }

        temp->next = newNode;
    }

    printf("Roll number %d inserted at end.\n", roll);
}

/* Search for a roll number */
void search(int roll) {
    struct Node *temp = head;

    while (temp != NULL) {
        if (temp->roll == roll) {
            printf("Roll number %d found in the list.\n", roll);
            return;
        }

        temp = temp->next;
    }

    printf("Roll number %d not found in the list.\n", roll);
}

/* Delete a roll number */
void deleteRoll(int roll) {
    struct Node *temp = head;
    struct Node *prev = NULL;

    if (head == NULL) {
        printf("List is empty.\n");
        return;
    }

    /* If the first node has the roll number */
    if (head->roll == roll) {
        head = head->next;
        free(temp);

        printf("Roll number %d deleted.\n", roll);
        return;
    }

    /* Search for the roll number */
    while (temp != NULL && temp->roll != roll) {
        prev = temp;
        temp = temp->next;
    }

    /* Roll number not found */
    if (temp == NULL) {
        printf("Roll number %d not found. Cannot delete.\n", roll);
        return;
    }

    prev->next = temp->next;
    free(temp);

    printf("Roll number %d deleted.\n", roll);
}

/* Display the list */
void display() {
    struct Node *temp = head;

    if (head == NULL) {
        printf("List is empty.\n");
        return;
    }

    printf("Updated Roll Numbers: ");

    while (temp != NULL) {
        printf("%d -> ", temp->roll);
        temp = temp->next;
    }

    printf("NULL\n");
}

/* Main function */
int main() {
    int choice, roll;

    while (1) {
        printf("\n--- Student Roll Number List ---\n");
        printf("1. Insert at Beginning\n");
        printf("2. Insert at End\n");
        printf("3. Search Roll Number\n");
        printf("4. Delete Roll Number\n");
        printf("5. Display List\n");
        printf("6. Exit\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {

            case 1:
                printf("Enter roll number: ");
                scanf("%d", &roll);
                insertBeginning(roll);
                display();
                break;

            case 2:
                printf("Enter roll number: ");
                scanf("%d", &roll);
                insertEnd(roll);
                display();
                break;

            case 3:
                printf("Enter roll number to search: ");
                scanf("%d", &roll);
                search(roll);
                break;

            case 4:
                printf("Enter roll number to delete: ");
                scanf("%d", &roll);
                deleteRoll(roll);
                display();
                break;

            case 5:
                display();
                break;

            case 6:
                printf("Program terminated.\n");
                return 0;

            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}