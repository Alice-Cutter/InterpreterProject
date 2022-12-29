#include <stdlib.h>
#include "value.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>


Value *activeMemory = NULL;
// Replacement for malloc that stores the pointers allocated. It should store
// the pointers in some kind of list; a linked list would do fine, but insert
// here whatever code you'll need to do so; don't call functions in the
// pre-existing linkedlist.h. Otherwise you'll end up with circular
// dependencies, since you're going to modify the linked list to use talloc.
void *talloc(size_t size){
        if (activeMemory == NULL) { 
            Value *myNull = malloc(sizeof(Value)); 
            myNull -> type = NULL_TYPE;
            Value *firstVal = malloc(sizeof(Value));
            firstVal -> p = malloc(size);
            Value *myCons = malloc(sizeof(Value));
            myCons -> type = CONS_TYPE;
            myCons -> c.car = firstVal;
            myCons -> c.cdr = myNull;
            activeMemory = myCons;
            return firstVal -> p;

        } else {
            Value *nextVal = malloc(sizeof(Value));
            nextVal -> p = malloc(size);
            Value *myCons = malloc(sizeof(Value));
            myCons -> type = CONS_TYPE;
            myCons -> c.car = nextVal;
            myCons -> c.cdr = activeMemory;
            activeMemory = myCons;
            return nextVal -> p;
        }
}

// Free all pointers allocated by talloc, as well as whatever memory you
// allocated in lists to hold those pointers.
void tfree(){
    while(activeMemory -> type != NULL_TYPE){
        Value *temp = activeMemory -> c.cdr; 
        free(activeMemory -> c.car -> p); //free string 
        free(activeMemory -> c.car);
        free(activeMemory);
        activeMemory = temp; 
    }
    free(activeMemory);
    activeMemory = NULL;
}

// Replacement for the C function "exit", that consists of two lines: it calls
// tfree before calling exit. It's useful to have later on; if an error happens,
// you can exit your program, and all memory is automatically cleaned up.
void texit(int status){
    tfree();
    exit(status);
}

