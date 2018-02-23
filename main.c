/* Created by: James Hamilton McRoberts IV
   Date:       February 19th, 2018

   Description:
        A Brainfuck (created by Urban Müller) interpreter written in C with
        simplicity and usability in mind. Licence included in repository.
 */

#include "bfHead.h"

void printHelp();
// Sets up interpritation
int bfBegin(bool e, char *out, const char *in);
// Does interpritation
int bfInit(bool e, char *out, exArray *prog);

int main(int argc, const char *argv[]) {
    /*if (argc == 1) {
        fprintf(stderr, "Not enough arguments.\n");
        return -1;
    }*/

    // Options used throughout program
    bool encode = TRUE;
    char *outputPtr = NULL, *inputPtr = NULL;

    // Parses options and argument
    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)) {
            printHelp();
            return 0;
        }
        else if ((strcmp(argv[i], "-n") == 0) || (strcmp(argv[i], "--no-encode") == 0)) {
            encode = FALSE;
        }
        else if ((strcmp(argv[i], "-o") == 0) || (strcmp(argv[i], "--output") == 0)) {
            // Not checking for "-" at start of name to do the possibility to do so.
            // Bad practice does not mean it is not practiced.
            if (i == argc - 1) {
                fprintf(stderr, "No file specified for output.\n");
                return -1;
            }

            outputPtr = argv[i + 1];
        }
        else {
            inputPtr = argv[i];
        }
    }

    // Checks to see if an input file was given
    if (!inputPtr) {
        fprintf(stderr, "No file specified for input.\n");
        return -1;
    }

    // Handles the language
    return bfBegin(encode, outputPtr, inputPtr);
}

void printHelp() {
    printf("Usage: bf [options] [file]\n\n");

    printf("Options and arguments:\n");
    printf("\t -h/--help\t: Print this help text.\n");
    printf("\t -n/--no-encode\t: No ASCII encoding on terminal output.\n");
    printf("\t -o/--output\t: Output the tape to a file.\n");
}

// Checks to see if the parsed char is a valid command for the language
bool validChar(char target) {
    return ((target == '>') || (target == '<') ||
            (target == '+') || (target == '-') ||
            (target == '.') || (target == ',') ||
            (target == '[') || (target == ']'));
}

// Sets up interpritation
int bfBegin(bool e, char *out, const char *in) {
    // Opens file to be loaded into memory
    FILE *input = fopen(in, "r");
    if (!input) {
        fprintf(stderr, "General IO failure with input.\n");
        return -1;
    }

    // Initializes area for program in memory
    exArray *prog = malloc(sizeof(exArray));
    initArray(prog, DEFAULT_SIZE);

    // Loads the program into memory
    char workingChar;
    while ((workingChar = (char)getc(input)) != EOF)
        if (validChar(workingChar))
            appendArray(prog, workingChar);

    // Closes file opened for program loading
    fclose(input);

    // Interprets language
    return bfInit(e, out, prog);
}

// Does interpritation
int bfInit(bool e, char *out, exArray *prog) {
    char tape[TAPE_LENGTH] = {0};
    char *ptr = tape;

    exArray *jmp = malloc(sizeof(exArray));
    initArray(jmp, DEFAULT_SIZE);

    for (int i = 0; i < prog->used; i++) {
        if (getArray(prog, i) == '>') {
            if (++ptr >= tape + TAPE_LENGTH) {
                fprintf(stderr, "Index greater than tape length: %i\n", TAPE_LENGTH);
            }
        }

        else if (getArray(prog, i) == '<') {
            if (--ptr < tape) {
                fprintf(stderr, "Negative tape index value.\n");
                exit(1);
            }
        }

        else if (getArray(prog, i) == '+')
            ++*ptr;

        else if (getArray(prog, i) == '-')
            --*ptr;

        else if (getArray(prog, i) == '.') {
            // Prints output depending on if the program is set to encode or not
            if (e)
                printf("%c", *ptr);
            else
                printf("%i ", *ptr);
        }

        else if (getArray(prog, i) == ',')
            *ptr = getchar();

        else if (getArray(prog, i) == '[') {
            if (*ptr) {
                appendArray(jmp, i);
            }
            else {
                int depth = 0, j = i;
                for (; j < prog->used; j++) {
                    if (getArray(prog, j) == '[')
                        depth++;

                    else if (getArray(prog, j) == ']')
                        if (--depth == 0) {
                            i = j;
                            j = prog->used + 1;
                        }
                }

                if (j == prog->used) {
                    fprintf(stderr, "Unaccounted for start bracket: Command #%i\n",
                            i + 1);
                    exit(1);
                }
            }
        }

        else if (getArray(prog, i) == ']') {
            if (jmp->used == 0) {
                fprintf(stderr, "Unaccounted for end bracket: Command #%i", i + 1);
                exit(1);
            }

            i = arrayPop(jmp) - 1;
        }
    }

    if (out) {
        FILE *output = fopen(out, "w");
        if (!output) {
            fprintf(stderr, "General IO failure with output.");
            return -1;
        }

        for (int i = 0; i < TAPE_LENGTH / OUT_ROW_WIDTH; i++) {
            for (int j = 0; j < OUT_ROW_WIDTH; j++)
                fprintf(output, " %i", tape[(i * OUT_ROW_WIDTH) + j]);

            fprintf(output, "\n");
        }

        fclose(output);
    }

    return 0;
}
