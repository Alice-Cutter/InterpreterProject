#include "value.h"
#include "linkedlist.h"
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "talloc.h"

bool stripComment(char charRead){
    while (charRead != EOF) {
        if (charRead == '\n') {
            return true; 
        }
        charRead = (char)fgetc(stdin);
    }
    return false;
}

bool isLetter(char charRead){
    if (charRead == 'A' || charRead == 'B' || charRead == 'C' || charRead == 'D' || charRead == 'E' || charRead == 'F' || charRead == 'G' || charRead == 'H' || charRead == 'I' || charRead == 'J' || charRead == 'K' || charRead == 'L' || charRead == 'M' || charRead == 'N' || charRead == 'O' || charRead == 'P' || charRead == 'Q' || charRead == 'R' || charRead == 'S' || charRead == 'T' || charRead == 'U' || charRead == 'V' || charRead == 'W' || charRead == 'X' || charRead == 'Y' || charRead == 'Z') {
        return true;
    }
    if (charRead == 'a' || charRead == 'b' || charRead == 'c' || charRead == 'd' || charRead == 'e' || charRead == 'f' || charRead == 'g' || charRead == 'h' || charRead == 'i' || charRead == 'j' || charRead == 'k' || charRead == 'l' || charRead == 'm' || charRead == 'n' || charRead == 'o' || charRead == 'p' || charRead == 'q' || charRead == 'r' || charRead == 's' || charRead == 't' || charRead == 'u' || charRead == 'v' || charRead == 'w' || charRead == 'x' || charRead == 'y' || charRead == 'z') {
        return true;
    }
    return false;
}

bool isSymbol(char charRead){
    if (charRead == '!' || charRead == '$' || charRead == '%' || charRead =='&' || charRead == '/' || charRead == ':' || charRead == '<' || charRead == '=' || charRead == '>' || charRead == '?' || charRead == '~' || charRead == '_' || charRead == '^' || charRead == '*') {
        return true;
    }
    return false;
}

bool isDigit(char charRead){
    if (charRead == '0' || charRead == '1' || charRead == '2' || charRead == '3' || charRead == '4' || charRead == '5' || charRead == '6' || charRead == '7' || charRead == '8' || charRead == '9') {
        return true;
    }
    return false;
}

Value *boolFind(char charRead){
    char charRead2 = (char)fgetc(stdin);  
    if(charRead2 == 't' || charRead2 == 'f'){ 
        Value *nextVal = talloc(sizeof(Value)); 
        nextVal -> type = BOOL_TYPE;
        if (charRead2 == 'f') {
            nextVal -> i = 0;
        } else {
            nextVal -> i = 1;
        }
        return nextVal;
    }
    printf("Syntax error: hashtag not followed by t or f\n");
    texit(0);  
    return NULL; 
}

Value *quoteFind (char charRead){
    char charRead2 = (char)fgetc(stdin);    
    int count = 0;
    char *quote = talloc(301 * sizeof(char));
    if(charRead2 == '\"'){
        Value *nextVal = talloc(sizeof(Value)); 
        nextVal -> type = STR_TYPE; 
        quote[0]='\0';
        nextVal -> s = quote; 
        return nextVal;  
    }
    while (charRead2 != EOF){
        //check if it's a double quote 
        if( charRead2 == '\"'){
            //create a node with the value of quote
            // return value node 
            Value *nextVal = talloc(sizeof(Value)); 
            nextVal -> type = STR_TYPE; 
            quote[count]='\0';
            nextVal -> s = quote; 
            return nextVal; 
        }else{
            //add charREad2 to quote 
            quote[count]=charRead2; 
            count++;
        }
        charRead2 = (char)fgetc(stdin);
    }   
    // If we are down here then we never found an end quote.
    printf("Syntax error: did not find close quotation\n");
    texit(0);
    return NULL;   
}

Value *findSymbol(char charRead){
    int count = 0;
    char *varName = talloc(301 * sizeof(char));
    varName[count]=charRead; 
    count++;
    char charRead2 = (char)fgetc(stdin); 
    while (charRead2 != ' ' && charRead2 != ')' && charRead2 != EOF && charRead2 != '\n'){
        if (isLetter(charRead2) ||  isSymbol(charRead2) || charRead2 == '.' || charRead2 == '+' || charRead2 == '-' || isDigit(charRead2)) {
            varName[count]=charRead2; 
            count++;
        // } else if (charRead2 == '.' || charRead2 == '+' || charRead2 == '-') {
        //     varName[count]=charRead2; 
        //     count++;
        //     char charRead3 = (char)fgetc(stdin); 
        //     if (charRead3 == ' ' || charRead3 != ')') {
        //         ungetc(charRead3, stdin);
        //         //create node
        //         Value *newVal = talloc(sizeof(Value)); 
        //         newVal -> type = SYMBOL_TYPE;
        //         char *var = talloc(301 * sizeof(char));
        //         varName[count]='\0';
        //         strcpy(var, varName); 
        //         newVal -> s = var; 
        //         return newVal; 
           
        //     } else {
        //         printf("Syntax error: can not have variable of that name\n");
        //         texit(0); 
        //     }
        } else {
            printf("Syntax error: variable used an invalid symbol:%c\n", charRead2);
            texit(0); 
        }
        charRead2 = (char)fgetc(stdin); 
    }
    ungetc(charRead2, stdin);
    Value *newVal = talloc(sizeof(Value)); 
    newVal -> type = SYMBOL_TYPE;
    varName[count]='\0';
    char *var = talloc(301 * sizeof(char));
    strcpy(var, varName); 
    newVal -> s = var; 
    
    return newVal; 
           
}

Value *findNumber(char charRead){
    int count = 0;
    bool dotBool = false;
    if (charRead == '.') {
        dotBool = true;
    }
    char *numName = talloc(301 * sizeof(char));
    numName[count]=charRead; 
    count++;
    char charRead2 = (char)fgetc(stdin); 
    while (charRead2 != ' ' && charRead2 != ')' && charRead2 != EOF && charRead2 != '\n'){
        if (isDigit(charRead2)){
            numName[count]=charRead2;  
            count++;
        } else if (charRead2 == '.' && dotBool == false){
            numName[count]=charRead2; 
            count++;
            dotBool = true;
        } else if (charRead2 == '.' && dotBool == true){
            printf("Syntax error: double dot in number\n");
            texit(0); 
        } else {
            printf("Syntax error: invalid character in number, \"%c\" \n", charRead2);
            
            texit(0); 
        }
        charRead2 = (char)fgetc(stdin); 
    }
    numName[count]='\0';
    ungetc(charRead2, stdin);
    if (dotBool == true) {
        Value *newVal = talloc(sizeof(Value)); 
        newVal -> type = DOUBLE_TYPE;
        newVal -> d = strtod(numName, NULL); 
        return newVal; 
    } else {
        Value *newVal = talloc(sizeof(Value)); 
        newVal -> type = INT_TYPE;
        newVal -> i = strtol(numName, NULL, 0);
        return newVal; 
    }
}


// Read all of the input from stdin, and return a linked list consisting of the
// tokens.
Value *tokenize(){
    char charRead;
    Value *list = makeNull();
    charRead = (char)fgetc(stdin);

    while (charRead != EOF) { 
        if (charRead == ' ') { //do nothing
        } else if (charRead == ';') {
            char charRead2 = (char)fgetc(stdin);
            bool use = stripComment(charRead2);
        } else if (charRead == '\n') {
        } else if (charRead == '\"') {
               Value *newVal = quoteFind(charRead); //outsource to method that does a string until closing double quote
               list = cons(newVal, list); 
        } else if (charRead == '(') {
            //create node with open type
            //outsource into symbol helper function

            Value *newVal = talloc(sizeof(Value)); 
            newVal -> type = OPEN_TYPE;
            list = cons(newVal, list);
        } else if (charRead == '#'){
                Value *newVal = boolFind(charRead); //outsource to bool helper function
                list = cons(newVal, list);
        } else if (charRead == ')'){
            Value *newVal = talloc(sizeof(Value)); 
            newVal -> type = CLOSE_TYPE;
            list = cons(newVal, list);
        } else if  (charRead == '+' || charRead == '-') {
            char charRead2 = (char)fgetc(stdin);
            if (isDigit(charRead2) || charRead == '.') {
                ungetc(charRead2, stdin);
                Value *newVal = findNumber(charRead);
                list = cons(newVal, list); 
            } else {
                ungetc(charRead2, stdin);
                Value *newVal = findSymbol(charRead);
                list = cons(newVal, list); 
            }
        } else if (isDigit(charRead) || charRead == '.') {
            Value *newVal = findNumber(charRead);
            list = cons(newVal, list); 
        } else if (isSymbol(charRead) || isLetter(charRead)) {
            Value *newVal = findSymbol(charRead);
            list = cons(newVal, list); 
        } else {
            printf("Syntax error: invalid start character %c\n", charRead);
            texit(0);
        }
        charRead = (char)fgetc(stdin);
    }
    Value *revList = reverse(list);
    return revList;
}

// Displays the contents of the linked list as tokens, with type information
void displayTokens(Value *list){
    while (list -> type != NULL_TYPE) {
        switch (list -> c.car -> type) {
            case OPEN_TYPE:
                printf("(:open\n");
                break;
            case CLOSE_TYPE:
                printf("):close\n");
                break;
            case PRIMITIVE_TYPE:
                break;
            case BOOL_TYPE:
                if (list -> c.car -> i == 1) {
                    printf("#t:boolean\n");
                } if (list -> c.car -> i == 0) {
                    printf("#f:boolean\n");
                }
                break;
            case SYMBOL_TYPE:
                printf("%s:symbol\n", list -> c.car -> s);
                break;
            case INT_TYPE:
                printf("%i:integer\n", list -> c.car -> i);
                break;
            case DOUBLE_TYPE:
                printf("%f:double\n", list -> c.car -> d);
                break;
            case STR_TYPE:
                printf("\"%s\":string\n", list -> c.car -> s);
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
        }
        list = list -> c.cdr;
    }
}


