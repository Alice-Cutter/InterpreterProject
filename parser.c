#include "value.h"
#include "linkedlist.h"
#include "tokenizer.h"
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "talloc.h"

// Takes a list of tokens from a Scheme program, and returns a pointer to a
// parse tree representing that program.
Value *parse(Value *tokens){
    int treeDepth = 0;
    Value *stack = makeNull();
    while (tokens -> type != NULL_TYPE) {
        Value *token = tokens -> c.car;
        if (token -> type == CLOSE_TYPE) {
            Value *tempList = makeNull();
            //do i need to do checks for cons type here?
            while (stack -> c.car -> type != OPEN_TYPE) {
                tempList = cons(stack -> c.car, tempList);
                stack = stack -> c.cdr;
                if (stack -> type == NULL_TYPE) {
                    printf("Syntax error: too many close parentheses\n");
                    texit(0);
                }
            } 
            stack = stack -> c.cdr; //skip it if it is an open type
            stack = cons(tempList, stack); //push that list back onto stack
            treeDepth = treeDepth - 1;
        } else {
            if (token -> type == OPEN_TYPE) {
                treeDepth++;
            }
            stack = cons(token, stack);
        }
        tokens = tokens -> c.cdr;
    }
    if (treeDepth > 0) {
        printf("Syntax error: not enough close parentheses\n");
        texit(0);
    } 
    if (treeDepth < 0) {
        printf("Syntax error: too many close parentheses\n");
        texit(0);
    } 
    stack = reverse(stack);
    return stack;

}

void printTreeHelper (Value *tree) {
    switch (tree -> type) {
            case OPEN_TYPE:
                printf("should not have an open\n");
                break;
            case CLOSE_TYPE:
                printf("should not have an close\n");
                break;
            case PRIMITIVE_TYPE:
                break;
            case BOOL_TYPE:
                if (tree -> i == 1) {
                    printf("#t ");
                } if (tree -> i == 0) {
                    printf("#f ");
                }
                break;
            case SYMBOL_TYPE:
                printf("%s ", tree -> s);
                break;
            case INT_TYPE:
                printf("%i ", tree -> i);
                break;
            case DOUBLE_TYPE:
                printf("%f ", tree -> d);
                break;
            case STR_TYPE:
                printf("\"%s\" ", tree -> s);
                break;
            case CONS_TYPE:
                if (tree -> c.car -> type == CONS_TYPE) {
                    printf("( ");
                }
                if (tree -> c.car -> type == NULL_TYPE) {
                    printf("( ");
                }
                printTreeHelper(tree -> c.car);
                printTreeHelper(tree -> c.cdr);
                break;
            case NULL_TYPE:
                printf(") ");
                break;
            case PTR_TYPE:
                break;
             case VOID_TYPE:
                break; 
            case CLOSURE_TYPE: 
                break; 
        }
}


// Prints the tree to the screen in a readable fashion. It should look just like
// Scheme code; use parentheses to indicate subtrees.
void printTree(Value *tree){
    Value *tempTree = tree;
    
    while(tempTree -> type != NULL_TYPE) {
        if (tempTree -> c.car -> type == CONS_TYPE) {
                    printf("( ");
        }
        if (tempTree -> c.car -> type == NULL_TYPE) {
                    printf("( ");
        }
        printTreeHelper(tempTree -> c.car);
        tempTree = tempTree -> c.cdr;
    }          
}

