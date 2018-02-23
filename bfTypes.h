/* Created by: James Hamilton McRoberts IV
   Date:       February 19th, 2018

   Description:
        Data types and functions used for those data types used in BFPL. These
        were used for simplicity, easy of implementation, and efficiency. License
        is included in the repository.
*/

// Easy boolean type
typedef char bool;
#define TRUE 1
#define FALSE 0


// Expandable array
typedef struct {
    int *payload;
    size_t used;
    size_t size;
} exArray;

// Starts expandable array
void initArray(exArray *target, size_t startingSize) {
    target->payload = (int *)malloc(startingSize * sizeof(int));
    target->used = 0;
    target->size = startingSize;
}

// Inserts value into expandable array (no shifting)
void insertArray(exArray *target, size_t index, int element) {
    if (index >= target->size) {
        target->used = index + 1;
        target->size = target->used * 2;
        target->payload = (int *)realloc(target->payload, target->size * sizeof(int));
    }
    else if (target->used == target->size) {
        target->size *= 2;
        target->payload = (int *) realloc(target->payload, target->size * sizeof(int));
    }
    else if (index + 1 > target->used) {
        target->used = index + 1;
    }

    target->payload[target->used++] = element;
}

// Adds value to the end of the array
void appendArray(exArray *target, int element) {
    if (target->used == target->size) {
        target->size *= 2;
        target->payload = (int *)realloc(target->payload, target->size * sizeof(int));
    }

    target->payload[target->used++] = element;
}

// Pops the top off of the array
int arrayPop(exArray *target) {
    if (target->used == 0) {
        fprintf(stderr, "exArray is empty and cannot be popped");
        exit(-1);
    }

    return target->payload[--target->used];
}

// Gets value from the array with some error logic
int getArray(exArray *target, size_t index) {
    if (index >= target->used) {
        fprintf(stderr, "exArray index out of bounds.\n");
        return target->payload[target->used - 1];
    }

    return target->payload[index];
}

// Clears array from memory
void freeArray(exArray *target) {
    free(target->payload);
    target->payload = NULL;
    target->used = 0;
    target->size = 0;
}