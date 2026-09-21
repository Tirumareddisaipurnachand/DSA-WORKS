/*An expression-processing application receives an arithmetic expression in infix form. Write a C
program using a stack to convert it to postfix form while correctly handling parentheses and
operator precedence for +, -, *, / and ^. Test the program using an expression containing multiple
operators and parentheses.*/
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX 100

char stack[MAX];
int top = -1;


void push(char ch)
{
    stack[++top] = ch;
}


char pop()
{
    return stack[top--];
}


int precedence(char ch)
{
    switch (ch)
    {
        case '^':
            return 3;
        case '*':
        case '/':
            return 2;
        case '+':
        case '-':
            return 1;
        default:
            return 0;
    }
}


int isRightAssociative(char ch)
{
    return ch == '^';
}

int main()
{
    char infix[MAX], postfix[MAX];
    int i, j = 0;
    char ch;

    printf("Enter an infix expression: ");
    scanf("%s", infix);

    for (i = 0; i < strlen(infix); i++)
    {
        ch = infix[i];

        
        if (isalnum(ch))
        {
            postfix[j++] = ch;
        }

        
        else if (ch == '(')
        {
            push(ch);
        }

        
        else if (ch == ')')
        {
            while (top != -1 && stack[top] != '(')
            {
                postfix[j++] = pop();
            }

            if (top != -1 && stack[top] == '(')
                pop();
        }

        
        else
        {
            while (top != -1 &&
                   stack[top] != '(' &&
                   (precedence(stack[top]) > precedence(ch) ||
                   (precedence(stack[top]) == precedence(ch) &&
                    !isRightAssociative(ch))))
            {
                postfix[j++] = pop();
            }

            push(ch);
        }
    }

    
    while (top != -1)
    {
        postfix[j = j+1] = pop();
    }

    postfix[j] = '\0';

    printf("Postfix expression: %s\n", postfix);

    return 0;
}