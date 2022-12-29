#include "value.h"
#include "parser.h"
#include "interpreter.h"
#include "linkedlist.h"
#include "tokenizer.h"
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "talloc.h"
#include <math.h>


void printTree3(Value *tree){
            switch(tree->type){
            case CONS_TYPE:  
                printf("(");
                while (tree -> type == CONS_TYPE){
                    printTree3(car(tree));
                    if (cdr(tree) -> type != CONS_TYPE && cdr(tree) -> type != NULL_TYPE) {
                        printf(" . ");
                        printTree3(cdr(tree));
                    }
                    tree = cdr(tree);
                }
                printf(")");
                break;
            case OPEN_TYPE:
                printf("(");
                break;
            case PRIMITIVE_TYPE:
                break;
            case CLOSE_TYPE:
                printf(")");
                break;
            case INT_TYPE:
                printf("%i",tree->i);
                break;
            case DOUBLE_TYPE:
                printf("%f",tree->d);
                break;
            case SYMBOL_TYPE:
                printf("%s", tree->s);
                break;
            case NULL_TYPE:
                printf("()");
                break;
            case BOOL_TYPE:
                if (tree->i == 1){
                    printf("#t");
                }
                else {
                    printf("#f");
                }
                break;
            case PTR_TYPE:
                printf("pointer");
                break;
            case STR_TYPE:
                printf("\"%s\"", tree->s);
                break;
            case VOID_TYPE:
                break; 
            case CLOSURE_TYPE:
                printf("#<procedure>");
                break; 
            default:
                printf("");
                break;
            }
            printf(" ");
}

Value *lookUpSymbol(Value *symbol, Frame *currentFrame){
    //starting in current frame, look through of bindingd and if it is there, return that value. 
    //otherwise, go up to parent frame and repat. do this until parent frame is null. 
    //If we have not found it, there are errors and this symbol has not been defined
    while(currentFrame != NULL){
       Value *currentBinding = currentFrame -> bindings; 
       while(currentBinding -> type != NULL_TYPE){
          if (!strcmp (car(car(currentBinding))->s , symbol->s)){
            // If the symbol is equal to the symbol of the current binding 
            return car(cdr(car(currentBinding))); 
         }
         else{
            currentBinding = cdr(currentBinding);
         }
       }
       currentFrame = currentFrame -> parent;
    }
    printf("Evaluation error: variable is not bound %s\n", symbol ->s);
    texit(0); 
    return NULL; // THIS IS A PLACEMENT LINE DELETE ME! 
}

Value *apply(Value *function, Value *args){
    Frame * newFrame = talloc(sizeof(Frame)); // creating a new frame 
    newFrame -> parent = function -> cl.frame; 
    Value *stop = makeNull(); 
    newFrame -> bindings = stop; 

    Value *currentParam = function -> cl.paramNames; //the param defined in the closure
    Value *currentArg = args; //the arguments given

    while(currentParam -> type != NULL_TYPE) {
        if (currentArg -> type == NULL_TYPE) {
            printf("Evaluation error: not enough arguments provided in function call");
            texit(0);
        }
        //add binding
        Value *endVal = makeNull();
        Value *lumped = cons(car(currentArg), endVal); 
        Value *lumped2 = cons(car(currentParam), lumped); 
        newFrame -> bindings = cons(lumped2, newFrame -> bindings); 

        currentParam = cdr(currentParam);
        currentArg = cdr(currentArg);
    }

    if (currentArg -> type != NULL_TYPE) {
            printf("Evaluation error: too many arguments provided in function call");
            texit(0);
    }
    return eval(function -> cl.functionCode, newFrame);

    //evaluate function body with new frame as environement
}

Value *applyPrimitive(Value *function, Value *args){
    Value *result = function -> pf(args);
    return result;
}

Value *evalSet(Value *variable, Value *expr, Frame *currentFrame){
    Value *evaledExpr = eval(expr, currentFrame);

    while(currentFrame != NULL){
       Value *currentBinding = currentFrame -> bindings; 
       while(currentBinding -> type != NULL_TYPE){
          if (!strcmp (car(car(currentBinding))->s , variable->s)){
            // If the symbol is equal to the symbol of the current binding 
            Value *consCell = cdr(car(currentBinding));
            consCell -> c.car = evaledExpr;
            Value * returnVal = talloc(sizeof(Value));
            returnVal -> type = VOID_TYPE; 
            returnVal -> s = variable -> s; 
            return returnVal; 
         }
         else{
            currentBinding = cdr(currentBinding);
         }
       }
       currentFrame = currentFrame -> parent;
    }
    printf("Evaluation error: variable is not bound %s\n", variable ->s);
    texit(0); 
    return NULL; 
}

Value *evalBegin(Value *args, Frame *currentFrame){
    Value *returnVal; 
    
    if(args -> type == NULL_TYPE){
        Value *voidVal = talloc(sizeof(Value)); 
        voidVal -> type = VOID_TYPE; 
        return voidVal; 
    }
  
     
    while( args -> type != NULL_TYPE){
        if( args -> type != CONS_TYPE){
            printf("Evaluation error; not enough arguments in evalBegin \n"); 
            texit(0); 
            
        }
        returnVal = eval(car(args), currentFrame); 
        args = cdr(args); 
    }
    return returnVal; 

}




Value *evalLetStar(Value *consList, Frame *parentFrame) {
    Frame *tempParent = parentFrame;

    Value *binding = cdr(consList);
    if (binding -> type != CONS_TYPE){
       printf("Evaluation error: binding in letstar is not of type CONS\n");
       texit(0);
    }
    Value *currentGrouping = car(binding);
    while (currentGrouping -> type != NULL_TYPE) {
       if (currentGrouping -> type != CONS_TYPE){
         printf("Evaluation error: group in letstar is not of type CONS\n");
         texit(0);
      }
      Value *v = car(currentGrouping);
      if (v -> type != CONS_TYPE){
         printf("Evaluation error: v  in letstar is not of type CONS\n");
         texit(0);
      }
      Value *e = cdr(v);
      if (e -> type != CONS_TYPE ){
         printf("Evaluation error:  e in letstar is not of type CONS\n");
         texit(0);
      }
      //check if the symbol is a duplicate
      Value *symbol = car(v);
      Value *evaluee = car(e);
      if (symbol -> type != SYMBOL_TYPE){
         printf("Evaluation error: symbol in letstar is not of type SYMBOL \n");
         texit(0);
      }
      Value *evaluated = eval(evaluee, tempParent);
      //add a binding
      Value *endVal = makeNull();
      Value *lumped = cons(evaluated, endVal); 
      Value *lumped2 = cons(symbol, lumped); 
      
      //new frame
      Frame *childFrame= talloc(sizeof(Frame));
      childFrame-> parent = tempParent; 
      Value *stop = makeNull();
      childFrame -> bindings = stop;
      childFrame -> bindings = cons(lumped2, childFrame -> bindings); 

      currentGrouping = cdr(currentGrouping);
      tempParent = childFrame;
    }

   //evaluate the body
    Value *body = cdr(binding);
    if(body -> type == NULL_TYPE){
        printf("Evaluation error: no args following the bindings in let.\n");
        texit(0);
    }

    Value *returnValue;

    while(body -> type != NULL_TYPE) {
        Value *firstBody = car(body);
        returnValue = eval(firstBody, tempParent);   
        body = cdr(body);
    }
    return returnValue;
}

Value *evalLetrec(Value *consList, Frame *parentFrame) {
 //1. Create a new frame env’ with parent env.
    Frame *childFrame= talloc(sizeof(Frame));
    childFrame-> parent = parentFrame; 
    Value *stop = makeNull();
    childFrame -> bindings = stop; // Initalize new frame where the parent is the paretn frame and it has no real value type 
    
   Value *symbolList = makeNull();
    Value *headTempList = makeNull(); 
    //now we start checking and evaluating the list
    Value *binding = cdr(consList);
    if (binding -> type != CONS_TYPE){
       printf("Evaluation error: binding is not of type CONS\n");
       texit(0);
    }
    Value *currentGrouping = car(binding);
    while (currentGrouping -> type != NULL_TYPE) {
       if (currentGrouping -> type != CONS_TYPE){
         printf("Evaluation error: group is not of type CONS\n");
         texit(0);
      }
      Value *v = car(currentGrouping);
      if (v -> type != CONS_TYPE){
         printf("Evaluation error: v or e is not of type CONS\n");
         texit(0);
      }
      Value *e = cdr(v);
      if (e -> type != CONS_TYPE ){
         printf("Evaluation error: v or e is not of type CONS\n");
         texit(0);
      }
      //check if the symbol is a duplicate
      Value *symbol = car(v);
      while (symbolList -> type != NULL_TYPE) {
         if (!strcmp(car(symbolList)->s , symbol->s)) {
               printf("Evaluation error: duplicate variable in let\n");
               texit(0);
         }
         symbolList = cdr(symbolList);
      }
      symbolList = cons(symbol, symbolList);

      Value *evaluee = car(e);
      if (symbol -> type != SYMBOL_TYPE){
         printf("Evaluation error: symbol is not of type SYMBOL \n");
         texit(0);
      }
      Value *evaluated = eval(evaluee, childFrame);
      //add a binding
      Value *endVal = makeNull();
      Value *lumped = cons(evaluated, endVal); 
      Value *lumped2 = cons(symbol, lumped); 
     headTempList  = cons(lumped2, headTempList); 

      currentGrouping = cdr(currentGrouping);
    }
    childFrame -> bindings = headTempList; 

   //evaluate the body
   Value *body = cdr(binding);
   if(body -> type == NULL_TYPE){
     printf("Evaluation error: no args following the bindings in let.\n");
     texit(0);
   }

   Value *returnValue;

   while(body -> type != NULL_TYPE) {
      Value *firstBody = car(body);
      returnValue = eval(firstBody, childFrame);   
      body = cdr(body);
   }
   return returnValue;

}



Value *evalLet(Value *consList, Frame *parentFrame) {
    //1. Create a new frame env’ with parent env.
    Frame *childFrame= talloc(sizeof(Frame));
    childFrame-> parent = parentFrame; 
    Value *stop = makeNull();
    childFrame -> bindings = stop; // Initalize new frame where the parent is the paretn frame and it has no real value type 
    
   Value *symbolList = makeNull();

    //now we start checking and evaluating the list
    Value *binding = cdr(consList);
    if (binding -> type != CONS_TYPE){
       printf("Evaluation error: binding is not of type CONS\n");
       texit(0);
    }
    Value *currentGrouping = car(binding);
    while (currentGrouping -> type != NULL_TYPE) {
       if (currentGrouping -> type != CONS_TYPE){
         printf("Evaluation error: group is not of type CONS\n");
         texit(0);
      }
      Value *v = car(currentGrouping);
      if (v -> type != CONS_TYPE){
         printf("Evaluation error: v or e is not of type CONS\n");
         texit(0);
      }
      Value *e = cdr(v);
      if (e -> type != CONS_TYPE ){
         printf("Evaluation error: v or e is not of type CONS\n");
         texit(0);
      }
      //check if the symbol is a duplicate
      Value *symbol = car(v);
      while (symbolList -> type != NULL_TYPE) {
         if (!strcmp(car(symbolList)->s , symbol->s)) {
               printf("Evaluation error: duplicate variable in let\n");
               texit(0);
         }
         symbolList = cdr(symbolList);
      }
      symbolList = cons(symbol, symbolList);

      Value *evaluee = car(e);
      if (symbol -> type != SYMBOL_TYPE){
         printf("Evaluation error: symbol is not of type SYMBOL \n");
         texit(0);
      }
      Value *evaluated = eval(evaluee, parentFrame);
      //add a binding
      Value *endVal = makeNull();
      Value *lumped = cons(evaluated, endVal); 
      Value *lumped2 = cons(symbol, lumped); 
      childFrame -> bindings = cons(lumped2, childFrame -> bindings); 

      currentGrouping = cdr(currentGrouping);
    }
   //evaluate the body
   Value *body = cdr(binding);
   if(body -> type == NULL_TYPE){
     printf("Evaluation error: no args following the bindings in let.\n");
     texit(0);
   }

   Value *returnValue;

   while(body -> type != NULL_TYPE) {
      Value *firstBody = car(body);
      returnValue = eval(firstBody, childFrame);   
      body = cdr(body);
   }
   return returnValue;

}

Value *evalQuote(Value *body, Frame *currentFrame){
    return car(body); 
}

Value *evalDefine(Value *variable, Value *expr, Frame *currentFrame) {
    Value * returnVal = talloc(sizeof(Value));
    returnVal -> type = VOID_TYPE; 
    returnVal -> s = variable -> s; 
    Value *evaluated = eval(expr, currentFrame);
    //Adding a binding

    Value *endVal = makeNull();
    Value *lumped = cons(evaluated, endVal); 
    Value *lumped2 = cons(variable, lumped); 
    currentFrame -> bindings = cons(lumped2, currentFrame -> bindings); 
    return returnVal; 
}

Value *evalLambda(Value *params, Value *body, Frame *currentFrame) {
    if(body -> type == NULL_TYPE){
        printf("Evaluation error: No body in lambda statement \n");
        texit(0); 
    }
    Value *paramList = makeNull();
    Value *currentParam = params;
    while (currentParam -> type != NULL_TYPE) {
        while (paramList -> type != NULL_TYPE) {
            if (!strcmp(car(currentParam)->s , car(paramList)->s)) {
                printf("Evaluation error: duplicate identifier in lambda. \n");
                texit(0); 
            }
            paramList = cdr(paramList);
        }
        if (car(currentParam)->type != SYMBOL_TYPE) {
            printf("Evaluation error: formal parameters for lambda must be symbols. \n");
            texit(0); 
        }
        paramList = cons(car(currentParam), paramList);
        currentParam = cdr(currentParam);
    }

    Value * closure = talloc(sizeof(Value));
    closure -> type = CLOSURE_TYPE; 
    closure -> cl.paramNames = params; //look at it
    closure -> cl.functionCode = body;
    closure -> cl.frame = currentFrame;
    return closure;
}

Value *evalAnd(Value *args, Frame *currentFrame) {
    Value *returnVal = talloc(sizeof(Value));
    returnVal -> type = BOOL_TYPE; 
    
    if(args -> type == NULL_TYPE){
        printf("Evaluation error: not enough arguments in and\n");
        texit(0);
    }
    if (car(args) -> type == NULL_TYPE){
        printf("Evaluation error: not enough arguments in and\n");
        texit(0);
    }

    Value *result;
    while (args -> type != NULL_TYPE) {
        if(args -> type != CONS_TYPE){
            printf("Evaluation error; no cons type in and \n"); 
            texit(0); 
        }
        result = eval(car(args), currentFrame);
        if (result -> type != BOOL_TYPE) {
            printf("Evaluation error; problem that argument in and does not eval to bool type \n"); 
            texit(0);
        }
        if (result -> i == 0) {
            returnVal -> i = 0;
            return returnVal;
        }
        args = cdr(args); 
    }

    returnVal -> i = 1;
    return returnVal;
}

Value *evalOr(Value *args, Frame *currentFrame) {
    Value *returnVal = talloc(sizeof(Value));
    returnVal -> type = BOOL_TYPE; 
    
    if(args -> type == NULL_TYPE){
        printf("Evaluation error: not enough arguments in or\n");
        texit(0);
    }
    if (car(args) -> type == NULL_TYPE){
        printf("Evaluation error: not enough arguments in or\n");
        texit(0);
    }

    Value *result;
    while (args -> type != NULL_TYPE) {
        if(args -> type != CONS_TYPE){
            printf("Evaluation error; no cons type in or \n"); 
            texit(0); 
        }
        result = eval(car(args), currentFrame);
        if (result -> type != BOOL_TYPE) {
            printf("Evaluation error; problem that argument in and does not eval to bool type \n"); 
            texit(0);
        }
        if (result -> i == 1) {
            returnVal -> i = 1;
            return returnVal;
        }
        args = cdr(args); 
    }

    returnVal -> i = 0;
    return returnVal;
}

Value *evalIf(Value *test, Value *consequent, Value *alternate, Frame *currentFrame){
   Value *testValue = eval(test, currentFrame);
   if ( testValue -> type == BOOL_TYPE){
      if(testValue -> i == 1 ){
         Value *consequentValue; 
         consequentValue = eval(consequent, currentFrame);  
         return consequentValue;
      }
      else{
         Value *alternateValue; 
         alternateValue = eval(alternate, currentFrame); 
         return alternateValue; 
      }
   }
   else {
      printf("Evaluation error: test of if statement did not evaluate to boolean type \n");
      exit(0);
   }
    return NULL; 
}

Value *evalCond(Value *args, Frame *currentFrame){
    Value *returnVal = talloc(sizeof(Value)); 
    returnVal -> type = VOID_TYPE;
    if(args -> type == NULL_TYPE){ 
        return returnVal; 
    }

    while (args -> type != NULL_TYPE){
        if(args -> type != CONS_TYPE){
            printf("Evaluation error: Not cons type in arguments for cond \n"); 
            texit(0); 
        }    
        if(car(args) -> type != CONS_TYPE){
            printf("Evaluation error: Not cons type in arguments for cond \n"); 
            texit(0); 
        }
        if(cdr(car(args)) -> type != CONS_TYPE){
            printf("Evaluation error: Not cons type in arguments for cond \n"); 
            texit(0); 
        }
        if(cdr(cdr(car (args ))) -> type != NULL_TYPE){
            printf("Evaluation error: Too many arguments for cond \n"); 
            texit(0); 
        }  

        Value *test1 = car(car(args)); 
        Value *consequent1 = car(cdr(car(args))); 

        //ELSE CHECK
        if (test1 -> type == SYMBOL_TYPE && !strcmp(test1->s,"else")) {
            return eval(consequent1, currentFrame); 
        }
        Value * evalue = eval(test1, currentFrame); 
        if(evalue -> i == 1){
            //we know it's true so we want to evaluate 
            return eval(consequent1, currentFrame); 
        }
        args = cdr(args); 

    }
    return returnVal;

}

Value *primitivePlus(Value *args) {
   int real = 0;
   double sum = 0;
   while (args -> type != NULL_TYPE) {
       if (args -> type != CONS_TYPE) {
           printf("Evaluation error: somethin fishy is going on in +\n");
            texit(0);
       }
        Value *currentArg = car(args); 
        if (currentArg -> type == INT_TYPE) {
            sum = sum + currentArg -> i;
        } else if (currentArg -> type == DOUBLE_TYPE) {
            real = 1;
            sum = sum + currentArg -> d;
        } else {
            printf("Evaluation error: non integer or real argument given in +\n");
            texit(0);
        }
        args = cdr(args);
   }
   Value *returnValue = talloc(sizeof(Value));
    if (real == 0) {
        returnValue -> type = INT_TYPE;
        returnValue -> i = sum;
        return returnValue;
    } else {
        returnValue -> type = DOUBLE_TYPE;
        returnValue -> d = sum;
        return returnValue;
    }
}

Value *primitiveMinus(Value *args) {
    int real = 0;
    double difference = 0;
    if (car(args) -> type == INT_TYPE) {
        difference = car(args) -> i;
    } else if (car(args) -> type == DOUBLE_TYPE) {
        real = 1;
        difference = car(args) -> d;
    } else {
        printf("Evaluation error: non integer or real argument given in -\n");
        texit(0);
    }
    Value *arg = cdr(args);
    while (arg -> type != NULL_TYPE) {
       if (arg -> type != CONS_TYPE) {
            printf("Evaluation error: somethin fishy is going on in -\n");
            texit(0);
       }
        Value *currentArg = car(arg); 
        if (currentArg -> type == INT_TYPE) {
            difference = difference - currentArg -> i;
        } else if (currentArg -> type == DOUBLE_TYPE) {
            real = 1;
            difference = difference - currentArg -> d;
        } else {
            printf("Evaluation error: non integer or real argument given in -\n");
            texit(0);
        }
        arg = cdr(arg);
   }

   Value *returnValue = talloc(sizeof(Value));
    if (real == 0) {
        returnValue -> type = INT_TYPE;
        returnValue -> i = difference;
        return returnValue;
    } else {
        returnValue -> type = DOUBLE_TYPE;
        returnValue -> d = difference;
        return returnValue;
    }

}

Value *primitiveMultiply (Value *args){
    int real = 0;
   double product;
     if (car(args) -> type == INT_TYPE) {
        product = car(args) -> i;
    } else if (car(args) -> type == DOUBLE_TYPE) {
        real = 1;
        product = car(args) -> d;
    } else {
        printf("Evaluation error: non integer or real argument given in * \n");
        texit(0);
    }

   args = cdr(args); 
   while (args -> type != NULL_TYPE) {
       if (args -> type != CONS_TYPE) {
           printf("Evaluation error: somethin fishy is going on in *\n");
            texit(0);
       }
        Value *currentArg = car(args); 
        if (currentArg -> type == INT_TYPE) {
            product = product * currentArg -> i;
        } else if (currentArg -> type == DOUBLE_TYPE) {
            real = 1;
            product = product * currentArg -> d;
        } else {
            printf("Evaluation error: non integer or real argument given in *\n");
            texit(0);
        }
        args = cdr(args);
   }
   Value *returnValue = talloc(sizeof(Value));
    if (real == 0) {
        returnValue -> type = INT_TYPE;
        returnValue -> i = product;
        return returnValue;
    } else {
        returnValue -> type = DOUBLE_TYPE;
        returnValue -> d = product;
        return returnValue;
    }
}

Value *primitiveDivide(Value *args){
    int real = 0;
    if (args -> type != CONS_TYPE) {
        printf("Evaluation error: divide does not have cons /\n");
        texit(0);
    }
    double firstVal;
    if (car(args) -> type == INT_TYPE) {
        firstVal = car(args) -> i;
    } else if (car(args) -> type == DOUBLE_TYPE) {
        real = 1;
        firstVal = car(args) -> d;
    }

    if (cdr(args) -> type != CONS_TYPE) {
        printf("Evaluation error: divide does not have cons /\n");
        texit(0);
    }

    double secondVal;
    if (car(cdr(args)) -> type == INT_TYPE) {
        secondVal = car(cdr(args)) -> i;
    } else if (car(cdr(args)) -> type ==DOUBLE_TYPE) {
        real = 1;
        secondVal = car(cdr(args)) -> d;
    }
    if (secondVal == 0) {
        printf("Evaluation error: cant divide by 0 \n");
        texit(0);
    }

    Value *returnValue = talloc(sizeof(Value));
    if (real == 0 && !(firstVal < secondVal)) {
        returnValue -> type = INT_TYPE;
        returnValue -> i = firstVal/secondVal;
        return returnValue;
    } else {
        returnValue -> type = DOUBLE_TYPE;
        returnValue -> d = firstVal/secondVal;
        return returnValue;
    }
}

Value *primitiveLess(Value *args){
    if (args -> type != CONS_TYPE) {
        printf("Evaluation error: less does not have cons /\n");
        texit(0);
    }

    double firstVal;
    if (car(args) -> type == INT_TYPE) {
        firstVal = car(args) -> i;
    } else if (car(args) -> type ==DOUBLE_TYPE) {
        firstVal = car(args) -> d;
    }
     if (cdr(args) -> type != CONS_TYPE) {
        printf("Evaluation error: less does not have cons /\n");
        texit(0);
    }

    double secondVal;
    if (car(cdr(args)) -> type == INT_TYPE) {
        secondVal = car(cdr(args)) -> i;
    } else if (car(cdr(args)) -> type ==DOUBLE_TYPE) {
        secondVal = car(cdr(args)) -> d;
    }

    Value *returnVal = talloc(sizeof(Value));
    returnVal -> type = BOOL_TYPE; 
    if(firstVal < secondVal){
        returnVal -> i = 1; 
    }
    else{
        returnVal -> i = 0; 
    }

    return returnVal;
}



Value *primitiveMore(Value *args){
    if (args -> type != CONS_TYPE) {
        printf("Evaluation error: more does not have cons /\n");
        texit(0);
    }

    double firstVal;
    if (car(args) -> type == INT_TYPE) {
        firstVal = car(args) -> i;
    } else if (car(args) -> type ==DOUBLE_TYPE) {
        firstVal = car(args) -> d;
    }
     if (cdr(args) -> type != CONS_TYPE) {
        printf("Evaluation error: more does not have cons /\n");
        texit(0);
    }

    double secondVal;
    if (car(cdr(args)) -> type == INT_TYPE) {
        secondVal = car(cdr(args)) -> i;
    } else if (car(cdr(args)) -> type ==DOUBLE_TYPE) {
        secondVal = car(cdr(args)) -> d;
    }

    Value *returnVal = talloc(sizeof(Value));
    returnVal -> type = BOOL_TYPE; 
    if(firstVal > secondVal){
        returnVal -> i = 1; 
    }
    else{
        returnVal -> i = 0; 
    }

    return returnVal;
}


Value *primitiveEqual(Value *args){
    if (args -> type != CONS_TYPE) {
        printf("Evaluation error: less does not have cons /\n");
        texit(0);
    }

    double firstVal;
    if (car(args) -> type == INT_TYPE) {
        firstVal = car(args) -> i;
    } else if (car(args) -> type ==DOUBLE_TYPE) {
        firstVal = car(args) -> d;
    }
     if (cdr(args) -> type != CONS_TYPE) {
        printf("Evaluation error: less does not have cons /\n");
        texit(0);
    }

    double secondVal;
    if (car(cdr(args)) -> type == INT_TYPE) {
        secondVal = car(cdr(args)) -> i;
    } else if (car(cdr(args)) -> type ==DOUBLE_TYPE) {
        secondVal = car(cdr(args)) -> d;
    }

    Value *returnVal = talloc(sizeof(Value));
    returnVal -> type = BOOL_TYPE; 
    if(firstVal == secondVal){
        returnVal -> i = 1; 
    }
    else{
        returnVal -> i = 0; 
    }

    return returnVal;
}

Value *primitiveModulo(Value *args){
    if (args -> type != CONS_TYPE) {
        printf("Evaluation error: modulo does not have cons /\n");
        texit(0);
    }

    int firstVal;
    if (car(args) -> type == INT_TYPE) {
        firstVal = car(args) -> i;
    }
    if (cdr(args) -> type != CONS_TYPE) {
        printf("Evaluation error: modulo does not have cons /\n");
        texit(0);
    }

    int secondVal;
    if (car(cdr(args)) -> type == INT_TYPE) {
        secondVal = car(cdr(args)) -> i;
    }

    Value *returnVal = talloc(sizeof(Value));
    returnVal -> type = INT_TYPE; 
    returnVal -> i = firstVal % secondVal;

    return returnVal;
}


Value *primitiveCar(Value *args) {
    if(args -> type != CONS_TYPE){
        printf("Evaluation error: not enough arguments in car \n");
        texit(0); 
    }
    if(cdr(args) -> type != NULL_TYPE){
        printf("Evaluation error: too many arguments in car \n"); 
        texit(0); 
    }
    if(car(args) -> type != CONS_TYPE) {
        printf("Evaluation error: argument given in car is not a list \n"); 
        texit(0); 
    }
    return car(car(args));
}
Value *primitiveCdr(Value *args) {
    if(args -> type != CONS_TYPE){
        printf("Evaluation error: not enough arguments in cdr \n");
        texit(0); 
    }
    if(cdr(args) -> type != NULL_TYPE){
        printf("Evaluation error: too many arguments in cdr \n"); 
        texit(0); 
    }
    if(car(args) -> type != CONS_TYPE) {
        printf("Evaluation error: argument given in cdr is not a list \n"); 
        texit(0); 
    } 
    return cdr(car(args));
}

Value *primitiveCons(Value *args) {
    if(args -> type != CONS_TYPE){
        printf("Evaluation error: not enough arguments in CONS\n");
        texit(0);
    }

    if(cdr(args) -> type != CONS_TYPE ){
        printf("Evaluation error: not enough arguments in CONS \n"); 
        texit(0); 
    }

    if(cdr(cdr(args))-> type != NULL_TYPE){
        printf("Evaluation error: too many arguments in CONS \n"); 
        texit(0); 
    }
    // WE know there are two arguments 

    Value *result = cons(car(args), car(cdr (args))); 
    return result; 
}

Value *primitiveNull(Value *args) {
    Value *returnValue = talloc(sizeof(Value));
    returnValue -> type = BOOL_TYPE;

    if(args -> type != CONS_TYPE){
        printf("Evaluation error: not enough arguments in null \n");
        texit(0); 
    
    }
    //WE are assuming that we can procede and the args is constype 
    Value * buddyVal = car(args); 
    if(cdr(args) -> type != NULL_TYPE){
        printf("Evaluation error: too many arguments in null \n"); 
        texit(0); 
    }

    if(buddyVal -> type == CONS_TYPE){
        if( car(buddyVal) -> type == NULL_TYPE && cdr(buddyVal) -> type == NULL_TYPE){
            returnValue -> i = 1; 

        }
        else{
            returnValue -> i = 0; 
        }
    
    }
    else{
        if (buddyVal -> type == NULL_TYPE){
            returnValue -> i = 1; //returning true; 
        }
        else{
            returnValue -> i = 0; //returning false
        }
    }    
   return returnValue;  
}

void bind(char *name, Value *(*function)(Value *), Frame *frame) {
    Value *symbol = talloc(sizeof(Value));
    symbol -> type = SYMBOL_TYPE;
    symbol -> s = name;

    Value *primitiveFunction = talloc(sizeof(Value));
    primitiveFunction -> type = PRIMITIVE_TYPE;
    primitiveFunction -> pf = function; //unsure about this syntax
    
    Value *endVal = makeNull();
    Value *lumped = cons(primitiveFunction, endVal); 
    Value *lumped2 = cons(symbol, lumped); 
    frame -> bindings = cons(lumped2, frame -> bindings);
}


Value *eval(Value *tree, Frame *frame) {
   switch (tree->type)  {
     case INT_TYPE: {
        return tree;
        break;
     }
     case DOUBLE_TYPE: {
        return tree;
        break;
     }  
      case BOOL_TYPE: {
        return tree;
        break;
     }  
      case STR_TYPE: {
        return tree;
        break;
     }  
       case PRIMITIVE_TYPE: {
        return tree;
        break;
     }  
     case OPEN_TYPE: {
        printf("Evaluation error: we should not have a open type");
        return tree;
        break;
     }
     case CLOSE_TYPE: {
        printf("Evaluation error: we should not have a close type");
        return tree;
        break;
     }
    case PTR_TYPE: {
        printf("Evaluation error: we should not have a ptr type");
        return tree;
        break;
     }
      case VOID_TYPE: {
        printf("Evaluation error: we should not have a void type");
        return tree;
        break;
     } case CLOSURE_TYPE: {
        printf("Evaluation error: we should not have a closure type");
        return tree;
        break;
     }
     case NULL_TYPE: {
        printf("Evaluation error: we should not have a null type");
        return tree;
        break;
     }
     case SYMBOL_TYPE: {
        Value *lookup = lookUpSymbol(tree, frame);
        return lookup;
        break;
     }  
     case CONS_TYPE: {
        Value *first = car(tree);
        Value *args = cdr(tree);
        Value *result;
        //if cons type, call eval on the car (first)
               // Sanity and error checking on first...
         //if first is of symbol type
        if (first -> type == SYMBOL_TYPE && !strcmp(first->s,"quote")) {
            if(args -> type == NULL_TYPE){
                printf("Evaluation error: quote had no args \n");
                texit(0); 
            }
            if(cdr(args) -> type != NULL_TYPE ){
                printf("Evaluation error: quote had too many args \n");
                texit(0); 
            }
            else{

                result = evalQuote(args, frame);
            }
        }
        else if (first -> type == SYMBOL_TYPE && !strcmp(first->s,"if")) {
            Value *test = args;
            if (test -> type != CONS_TYPE){
                printf("Evaluation error: test is not of type CONS\n");
                texit(0);
            }
            Value *consequent = cdr(test);
            if (consequent -> type != CONS_TYPE){
                printf("Evaluation error: consequent is not of type CONS\n");
                texit(0);
            }
            Value *alternate = cdr(consequent);
            if (alternate -> type != CONS_TYPE){
                printf("Evaluation error: alternate is not of type CONS\n");
                texit(0);
            }
            if (cdr(alternate) -> type != NULL_TYPE) {
                printf("Evaluation error: more than three arguments\n");
                texit(0);
            }
            result = evalIf(car(test), car(consequent), car(alternate), frame);
        } 
        else if (first -> type == SYMBOL_TYPE && !strcmp(first->s,"let")) {
            result = evalLet(tree, frame);
        } 
        else if (first -> type == SYMBOL_TYPE && !strcmp(first->s,"let*")) {
            result = evalLetStar(tree, frame);
        } 
        else if (first -> type == SYMBOL_TYPE && !strcmp(first->s,"letrec")) {
            result = evalLetrec(tree, frame);
        } 
        else if (first -> type == SYMBOL_TYPE && !strcmp(first->s,"lambda")) {
            Value *consParams = args;
            if (consParams -> type != CONS_TYPE){
                printf("Evaluation error: Consparams in lambda is not of type CONS\n");
                texit(0);
            }
            
            Value *params = car(consParams);
            Value *consBody = cdr(consParams);
            if (consBody -> type == CONS_TYPE) {
                if (cdr(consBody) -> type != NULL_TYPE) {
                    printf("Evaluation error: too many thingsin the body of a lambda");
                }
            }
            else if ( consBody -> type == NULL_TYPE){
                printf("Evaluation error: There is no body in lambda\n"); 
                texit(0); 
            }
            result = evalLambda(params, car(consBody), frame);
        } 
        else if (first -> type == SYMBOL_TYPE && !strcmp(first->s, "begin")){
            result = evalBegin(args, frame); 
        }
        else if (first -> type == SYMBOL_TYPE && !strcmp(first->s, "and")){
            result = evalAnd(args, frame); 
        }
        else if (first -> type == SYMBOL_TYPE && !strcmp(first->s, "or")){
            result = evalOr(args, frame); 
        }
        else if(first -> type == SYMBOL_TYPE && !strcmp(first -> s, "cond")){
            result = evalCond(args, frame); 
        }
        else if (first -> type == SYMBOL_TYPE && !strcmp(first->s,"define")) {
            Value *consVar = args;
            if (consVar -> type != CONS_TYPE){
                printf("Evaluation error: Consvar in define is not of type CONS\n");
                texit(0);
            }
            Value *variable = car(consVar);
            if (variable -> type != SYMBOL_TYPE){
                printf("Evaluation error: variable in define is not of type SYMBOL\n");
                texit(0);
            }
            Value *consExpr = cdr(consVar);
            if (consExpr -> type != CONS_TYPE){
                printf("Evaluation error: ConsExpr in define is not of type CONS\n");
                texit(0);
            }
            Value *expr = car(consExpr);
            if (cdr(consExpr) -> type != NULL_TYPE){
                printf("Evaluation error: too many arguments in define\n");
                texit(0);
            }
            result = evalDefine(variable, expr, frame);
        }
        else if (first -> type == SYMBOL_TYPE && !strcmp(first->s,"set!")) {
            Value *consVar = args;
            if (consVar -> type != CONS_TYPE){
                printf("Evaluation error: Consvar in set! is not of type CONS\n");
                texit(0);
            }
            Value *variable = car(consVar);
            if (variable -> type != SYMBOL_TYPE){
                printf("Evaluation error: variable in set! is not of type SYMBOL\n");
                texit(0);
            }
            Value *consExpr = cdr(consVar);
            if (consExpr -> type != CONS_TYPE){
                printf("Evaluation error: ConsExpr in set! is not of type CONS\n");
                texit(0);
            }
            Value *expr = car(consExpr);
            if (cdr(consExpr) -> type != NULL_TYPE){
                printf("Evaluation error: too many arguments in set!\n");
                texit(0);
            }
            result = evalSet(variable, expr, frame);
        }        // .. other special forms here...
        else {
            // RECURSIVE CALL OF EVALUATE! 
            Value *linkedList = makeNull(); 
            Value *evaluatedFunction = eval(first, frame);

            while(args -> type != NULL_TYPE){
                Value *evaluatedE = eval(car(args), frame);
                linkedList = cons(evaluatedE, linkedList);

                args = cdr(args);
            }
            linkedList = reverse(linkedList);

            if (evaluatedFunction -> type == PRIMITIVE_TYPE) {
                result = applyPrimitive(evaluatedFunction, linkedList);
            } else {
                result = apply(evaluatedFunction, linkedList);
            }
         }
        return result; 
        break;
     }
    }    
}


void interpret(Value *tree){
    Frame *globalFrame = talloc(sizeof(Frame));
    Value *endVal = makeNull();
    globalFrame -> bindings = endVal; 
    globalFrame -> parent = NULL;

    //do the bindings in the global frame
    bind("+", primitivePlus, globalFrame);
    bind("null?", primitiveNull, globalFrame);
    bind("car", primitiveCar, globalFrame);
    bind("cdr", primitiveCdr, globalFrame);
    bind("cons", primitiveCons, globalFrame);
    bind("-", primitiveMinus, globalFrame);
    bind("*", primitiveMultiply, globalFrame);
    bind("<", primitiveLess, globalFrame);
    bind(">", primitiveMore, globalFrame);
    bind("=", primitiveEqual, globalFrame);
    bind("/", primitiveDivide, globalFrame);
    bind("modulo", primitiveModulo, globalFrame);

    while (tree-> type != NULL_TYPE){
        Value *evaluated = eval(car(tree), globalFrame); 
        tree = tree -> c.cdr; 
        printTree3(evaluated); 
        printf("\n");
    }
}