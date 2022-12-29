#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "value.h"
#include <assert.h>
#include "talloc.h"

// Create a new NULL_TYPE value node. MAKING THE END OF LIST 
Value *makeNull(){
    Value *myNull = talloc(sizeof(Value)); 
    myNull -> type = NULL_TYPE;
    return myNull;
}

// Create a new CONS_TYPE value node.
Value *cons(Value *newCar, Value *newCdr){
    Value *myCons = talloc(sizeof(Value));
    myCons -> type = CONS_TYPE;
    myCons -> c.car = newCar;
    myCons -> c.cdr = newCdr;
    return myCons;
}

// Display the contents of the linked list to the screen in some kind of
// readable format
void display(Value *list){
    printf("[");
    while (list -> type != NULL_TYPE) {
        switch (list -> c.car -> type) {
            case INT_TYPE:
                printf("%i ", list -> c.car -> i);
                break;
            case PRIMITIVE_TYPE:
                break;
            case DOUBLE_TYPE:
                printf("%f ", list -> c.car -> d);
                break;
            case STR_TYPE:
                printf("%s ", list -> c.car -> s);
                break;
            case CONS_TYPE:
                break;
            case NULL_TYPE:
                break;
            case PTR_TYPE:
                break;
            case VOID_TYPE:
                break;
            case CLOSURE_TYPE:
                break;
            case OPEN_TYPE:
                break;
            case CLOSE_TYPE:
                break;
            case BOOL_TYPE:
                break;
            case SYMBOL_TYPE:
                break;
        }
        list = list -> c.cdr;
    }
    printf("]\n");
}

// Return a new list that is the reverse of the one that is passed in. All
// content within the list should be duplicated; there should be no shared
// memory whatsoever between the original list and the new one.
//
// FAQ: What if there are nested lists inside that list?
// ANS: There won't be for this assignment. There will be later, but that will
// be after we've got an easier way of managing memory.
Value *reverse(Value *list){
    if(list -> type == NULL_TYPE){
        return makeNull();
    }else{
        Value *newList = makeNull();  // should be the CDR each time for the rest of the list 
        while(list -> type != NULL_TYPE){
            newList = cons(list -> c.car, newList);
            list = list -> c.cdr;
        }
        return newList;

    }
}




// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car(Value *list){
    assert(list != NULL);
    assert(list -> type == CONS_TYPE);
    return list -> c.car;
}

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list){
    assert(list != NULL);
    assert(list -> type == CONS_TYPE);
    return list -> c.cdr;
}

// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value){
    assert(value != NULL);
    if (value -> type == NULL_TYPE) {
        return true;
    } else {
        return false;
    }
}

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value){
    int count = 0; 
    while(value -> type != NULL_TYPE){
        assert(value -> type == CONS_TYPE);
        count++; 
        value = value -> c.cdr; 

    }
    return count; 
}

