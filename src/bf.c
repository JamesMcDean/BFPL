/* Created by: James Hamilton McRoberts IV
   Date:       February 19th, 2018

   Description:
        A Brainfuck (created by Urban Müller) interpreter written in C with
        simplicity and usability in mind. Licence included in repository.
*/

#include "bfHead.h"



void printHelp();
// Sets up interpritation
void bfBegin(bool encoding, char *outputString, bool persistentOutput, const char *inputString);

int main(int argc, const char *argv[]) {
    // Options used throughout program
    bool encode = TRUE, persist = FALSE;
    char *outputPtr = NULL, *inputPtr = NULL;

    // Parses options and arguments
    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)) {
            printHelp();
            return EXIT_SUCCESS;
        }
        else if ((strcmp(argv[i], "-n") == 0) || (strcmp(argv[i], "--no-encode") == 0)) {
            encode = FALSE;
        }
        else if ((strcmp(argv[i], "-o") == 0) || (strcmp(argv[i], "--output") == 0)) {
            // Not checking for "-" at start of name to do the possibility to do so.
            // Bad practice does not mean it is not practiced.
            if (i == argc - 1) {
                fprintf(stderr, "No file specified for output.\n");
                return EXIT_FAILURE;
            }

            outputPtr = argv[i + 1];
        }
        else if (strcmp(argv[i], "-p") == 0) {
            persist = TRUE;
        }
        else {
            inputPtr = argv[i];
        }
    }

    // Checks to see if an input file was given
    if (!inputPtr) {
        fprintf(stderr, "No file specified for input.\n");
        return EXIT_FAILURE;
    }

    // Handles the language
    bfBegin(encode, outputPtr, persist, inputPtr);

    return EXIT_SUCCESS;
}

void printHelp() {
    printf("Usage: bf [options] [file]\n\n");

    printf("Options and arguments:\n");
    printf("\t -h/--help\t: Print this help text.\n");
    printf("\t -n/--no-encode\t: No ASCII encoding on terminal output.\n");
    printf("\t -o/--output\t: Output the tape to a file.\n");
    printf("\t -p\t: Persistent output (must have output set).");
}



// Does interpritation
void bfInit(bool encoding, bool persistentOutput, char *outputString, exArray *program);

// Checks to see if the parsed char is a valid command for the language
bool validChar(char target) {
    return ((target == '>') || (target == '<') ||
            (target == '+') || (target == '-') ||
            (target == '.') || (target == ',') ||
            (target == '[') || (target == ']'));
}

// Sets up interpritation
void bfBegin(bool encoding, char *outputString, bool persistentOutput, const char *inputString) {
    // Opens file to be loaded into memory
    FILE *input = fopen(inputString, "r");
    if (!input) {
        fprintf(stderr, "General IO failure with input.\n");
        exit(EXIT_FAILURE);
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
    bfInit(encoding, persistentOutput, outputString, prog);
}



// Outputs tape to file
int bfOut(char *tape, char *outputString) {
    FILE *output = fopen(outputString, "w");
    if (!output) {
        fprintf(stderr, "General IO failure with output.");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < TAPE_LENGTH / OUT_ROW_WIDTH; i++) {
        for (int j = 0; j < OUT_ROW_WIDTH; j++) {
            if (j == (OUT_ROW_WIDTH / 2) - 1)
                fprintf(output, " --");

            fprintf(output, " %i", tape[(i * OUT_ROW_WIDTH) + j]);
        }

        fprintf(output, "\n");
    }

    fclose(output);

    return EXIT_SUCCESS;
}

// Does interpritation
void bfInit(bool encoding, bool persistentOutput, char *outputString, exArray *program) {
    // Program memory
    char tape[TAPE_LENGTH] = {0};
    char *ptr = tape;

    // Holds jumps for looping commands
    exArray *jmp = malloc(sizeof(exArray));
    initArray(jmp, DEFAULT_SIZE);

    // Used to check to see if the output to file fails
    bool outFail = FALSE;

    for (int i = 0; i < program->used; i++) {
        if (getArray(program, i) == '>') {
            if (++ptr >= tape + TAPE_LENGTH) {
                fprintf(stderr, "Index greater than tape length: %i\n", TAPE_LENGTH);
                exit(EXIT_FAILURE);
            }
        }

        else if (getArray(program, i) == '<') {
            if (--ptr < tape) {
                fprintf(stderr, "Negative tape index value.\n");
                exit(EXIT_FAILURE);
            }
        }

        else if (getArray(program, i) == '+')
            ++*ptr;

        else if (getArray(program, i) == '-')
            --*ptr;

        else if (getArray(program, i) == '.') {
            // Prints output depending on if the program is set to encode or not
            if (encoding)
                printf("%c", *ptr);
            else
                printf("%i ", *ptr);
        }

        else if (getArray(program, i) == ',')
            *ptr = getchar();

        else if (getArray(program, i) == '[') {
            if (*ptr) {
                appendArray(jmp, i);
            }
            // Skips loop chunks
            else {
                int depth = 0, j = i;
                for (; j < program->used; j++) {
                    if (getArray(program, j) == '[')
                        depth++;

                    else if (getArray(program, j) == ']')
                        if (--depth == 0) {
                            i = j;
                            j = program->used + 1;
                        }
                }

                if (j == program->used) {
                    fprintf(stderr, "Unaccounted for start bracket: Command #%i\n",
                            i + 1);
                    exit(EXIT_FAILURE);
                }
            }
        }

        else if (getArray(program, i) == ']') {
            if (jmp->used == 0) {
                fprintf(stderr, "Unaccounted for end bracket: Command #%i", i + 1);
                exit(EXIT_FAILURE);
            }

            i = arrayPop(jmp) - 1;
        }

        // Stops persistent output if failure occurs
        if ((!outFail) && (persistentOutput && outputString))
            bfOut(tape, outputString);
        else
            outFail = TRUE;
    }

    // Output for lack of persistent option
    if ((!persistentOutput) && outputString)
        bfOut(tape, outputString);
}
