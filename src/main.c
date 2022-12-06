/* File Name: main.c
 * Created By: ZW
 * Created On: 2022-11-21
 * Purpose: main.c is the controller script and entry point 
 * for th fqsplit application. source code herein parses the 
 * commandline arguments and runs/ monitors the procedures
 */

/* library imports
 ----------------------------------------------------------------------------
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>


/* TYPES, STRUCTS, CONSTANTS, and MACROS definitions
 ----------------------------------------------------------------------------
 */

#define PARSE_ERROR_EXIT_CODE 1
#define FILE_OPEN_ERROR_EXIT_CODE 2
#define MAX_SPLITS_REQUEST_ERROR_EXIT_CODE 3
#define MIN_CMDARGS 2
#define INPUT_BUFFER_SIZE 3200  //input buffer size for reading lines from fastq


// Typedef for UserArgs. The structure that holds information
// about the passed commandline arguments.
typedef struct UserArgs {
    int nSplits;
    int bufferRecs;
    bool smkFormat;
    char *outputDir;
    char *outputBase;
    char *inputFastq;
} UserArgs;

// Typedef for FileInfo. FileInfo is a structure that holds information
// relating to a file for IO operations; 
typedef struct FileInfo {
    char *filePath;
    char *fileMode;
    FILE *fptr;
} FileInfo;


/* FUNCTION declarations
 ----------------------------------------------------------------------------
 */

/* Command Line Parsing functions */

// function declaration for help message printer. This routine takes no
// arguments and has no return value, it simply prints the commandline
// help message (either because a parsing error is raised or because
// the help flags [-h] [--help] were passed via the commandline
void print_helpmsg();

// function declaration for welcome message printer. This routine takes one
// arguments:
//      (1) uargs -> the userargs object with parsed content from the command line
// and has no return value, it simply prints a welcome message containing the
// passed arguments so that users can see in the logs the parameterization of the run
void print_welcomemsg(UserArgs uargs);

// function declaration for UserArgs printer. This routine takes a UserArgs
// object, and formats and prints it (mostly for debugging purposes).
// it has one argument:
//      (1) uargs -> the userargs object to print
void print_UserArgs(UserArgs uargs);

// function declaration for flag_parser: the function takes five
// arguments:
//      (1) shortflag -> the shorthand string of the flag to parse
//      (2) longflag -> the longhand string of the flag to parse
//      (3) failstr -> the string to be returned if the parser fails
//      (4) argv -> the raw string array passed by the commandline
//      (5) argc -> the raw argument count (length of argv)
// returns:
//      (1) a copy of the string which accompanied the parsed flag,
//          or the failstr string, if the passed flag cannot be
//          found in the input arguments.
char * flag_parser(char *shortflag, char *longflag,
            char *failstr, char **argv, int argc);

// function declaration for bool_parser: the function takes five
// arguments:
//      (1) shortflag -> the shorthand string of the flag to parse
//      (2) longflag -> the longhand string of the flag to parse
//      (3) argv -> the raw string array passed by the commandline
//      (4) argc -> the raw argument count (length of argv)
// returns:
//      (1) true if the switch flag is present in the commandline input,
//          or false if the passed switch flag cannot be
//          found in the input arguments.
bool switch_parser(char *shortflag, char *longflag, char **argv, int argc);

/* File Operations Input/Output functions */

// function declaration for file_open: the function, which initializes a
// passed FileInfo object. The FileInfo object needs to then be passed to the function
// file_close in order to close the file connetion and free the memory allocated
// to store the file path and mode. the function takes three arguments:
//      (1) fobj -> pointer to an declared FileInfo object
//      (2) fpath -> the file path of the file to open
//      (3) fmode -> the operation mode to open the file with (i.e. r, w, ..)
// returns:
//      (1) integer code, zero for success, 1 for failure if file cannot open
int file_open(FileInfo *fobj, char *fpath, char *fmode);

// function declaration for file_close: the function, which has no return,
// closes the file connection inside the passed FileObject, and frees allocated
// memory for the filePath and fileMode variables. The function takes one
// argument:
//      (1) pointer to FileInfo object containing the the opened
//          file connection and file path, and file mode
// returns: no return value. This function is necessary to prevent memory
// leaks and issues with closing file connections
void file_close(FileInfo * fobj);

// function declaration for build_outpath: the function which formats a 
// string representing the file path of the output fastq to be written
// the function takes as inputs, three arguments:
//      (1) pathstr -> a pointer to the string to hold the path
//      (2) uargs -> a copy of the user args structure containing passed params
//      (3) splitidx -> an integer representing the file number of the file to be
//          created. this builds the prefix which either is formatted using
//          smkFormat ({outputDir}/{splitidx}-of-{nSplits}.{outputBase}.fastq) or 
//          default   ({outputDir}/_{splitidx}.{outputBase}.fastq) 
// the function returns: no value
void build_outpath(char *pathstr, UserArgs uargs, int splitidx);

/* MAIN execution block
 ----------------------------------------------------------------------------
 */

int main(int argc, char **argv) {

    // time the main routine
    time_t start, end;
    time(&start);

    /* PARSE USER INPUTS AND PRINT WELCOME */
    /* ----------------------------------- */

    // check for help flags. If they are present, print the help
    // message and exit the program without executing.
    if(switch_parser("-h", "--help",argv, argc)){
        print_helpmsg();
        exit(PARSE_ERROR_EXIT_CODE);
    }

    // initialize the UserArgs object using the commandline parsers,
    // throw an error if too few arguments are passed to fqsplit
    UserArgs uargs;
    if (argc > (MIN_CMDARGS + 1 )) {
        // assign parsed command line values to UserArgs object
        uargs.nSplits = atoi(flag_parser("-n", "--n-splits", "5", argv, (argc - MIN_CMDARGS)));
        uargs.bufferRecs = atoi(flag_parser("-b", "--buffer-recs", "100", argv, (argc - MIN_CMDARGS)));
        uargs.smkFormat = switch_parser("-s", "--smk-format", argv, (argc - MIN_CMDARGS));
        uargs.outputDir = flag_parser("-o", "--outdir", ".", argv, (argc - MIN_CMDARGS));
        uargs.outputBase = argv[(argc - 2)]; // extract second to last argument
        uargs.inputFastq = argv[(argc - 1)]; // extract the last argument
    } else {
        printf("\nERROR: not enough arguments passed to fqsplit\n");
        print_helpmsg();
        exit(PARSE_ERROR_EXIT_CODE);
    }

    // print welcome message containing parsed arguments to user
    print_welcomemsg(uargs);

    /* OPEN INPUT FILE CONNECTION AND OUTPUT FILES */
    /* ------------------------------------------- */

    // open input fastq file connection throw error if we 
    // can't open the file connection
    FileInfo *inputfq = malloc(sizeof(FileInfo));    
    if (file_open(inputfq, uargs.inputFastq, "r") != 0) {
        printf("Error, could not open file connection to: %s\n", uargs.inputFastq);
        free(inputfq->filePath);
        free(inputfq->fileMode);
        free(inputfq);
        exit(FILE_OPEN_ERROR_EXIT_CODE);
    } else {
        printf("Opened Input File %s\n", inputfq->filePath);
    }

    // open array of FileInfo pointers for output files
    FileInfo **outFileInfoArr = malloc(uargs.nSplits * sizeof(FileInfo));
    char *fpath = malloc(PATH_MAX * sizeof(char));
    int i, j, fstatus = 0;
    if (uargs.nSplits < (FOPEN_MAX - 1)) {
        for (i = 0; i < uargs.nSplits; i++) { 
            build_outpath(fpath, uargs, i+1);
            printf("Opening File: %s for writing\n", fpath);
            outFileInfoArr[i] = malloc(sizeof(FileInfo));
            fstatus = file_open(outFileInfoArr[i], fpath, "w");  // open outfile objs
            if (fstatus != 0) {
                printf("Error. could not open file %s . Exiting..\n", fpath);
                free(outFileInfoArr[i]);
                for (j = (i - 1); j >= 0; j--) {
                    printf("Closing Output File: %s\n", outFileInfoArr[j]->filePath);
                    file_close(outFileInfoArr[j]);
                    free(outFileInfoArr[j]);
                }
                free(outFileInfoArr);
                file_close(inputfq);
                free(inputfq);
                free(fpath);
                exit(FILE_OPEN_ERROR_EXIT_CODE);  
            }
        }
    } else {
        printf("Error. too many files requested. --n-splits should"
               " be less than or equal to %d . Number or splits requested"
               " by the user: %d", FOPEN_MAX-1, uargs.nSplits);
        file_close(inputfq);
        free(inputfq);
        free(fpath);
        exit(MAX_SPLITS_REQUEST_ERROR_EXIT_CODE);
    }
    free(fpath);


    /* WRITE RECORDS TO OUTPUT FILES in ROUND ROBIN */
    /* -------------------------------------------- */

    // place fastq records into output files created above
    // each file recieves uargs.bufferRecs records before
    // rotating to the next file. 
    char input_buffer[INPUT_BUFFER_SIZE] = {'\0'}; 
    int lineIdx = 0, recIdx = 0, arrIdx = 0;
    while(fgets(input_buffer, INPUT_BUFFER_SIZE, inputfq->fptr) != NULL) {
        fputs(input_buffer, outFileInfoArr[arrIdx]->fptr);    
        
        lineIdx++;
        if (lineIdx % 4 == 0) {
            recIdx++;
        }
        if (recIdx == uargs.bufferRecs) {
            arrIdx = ((arrIdx + 1) % uargs.nSplits);
            recIdx = 0;
        }
    }


    /* CLEANUP AND EXIT WITHOUT ERROR */
    /* ------------------------------ */

    // close output file connections and free array
    for (i = 0; i < uargs.nSplits; i++) {
        printf("Closing Output File: %s\n", outFileInfoArr[i]->filePath);
        file_close(outFileInfoArr[i]);
        free(outFileInfoArr[i]);
    }
    free(outFileInfoArr);

    // close input fastq file connection after use
    file_close(inputfq);
    free(inputfq);

    // return without error code
    time(&end);
    printf("Elapsed Time: %d\n", end - start);
    return 0;
}


/* FUNCTION definitions
 ----------------------------------------------------------------------------
 */

/* Command Line Parsing functions */

// function definition for help message printer. This routine takes no
// arguments and has no return value, it simply prints the commandline
// help message (either because a parsing error is raised or because
// the help flags [-h] [--help] were passed via the commandline
void print_helpmsg() {
    printf(
        "\n./fqsplit usage:\n"
        "./fqsplit [OPTIONS] OUTPUT_BASENAME INPUT_FASTQ\n"
        "-----------------------------------------------\n\n"
        "arguments:\n"
        "-n, --n-splits\t\t\tNumber of files to split INPUT FASTQ into\n"
        "-b, --buffer-recs\t\tNumber of records to write before rotating between output files\n"
        "-s, --smk-format\t\tPrefix style for output files should be in the snakemake scatter style (etc. 1-of-n.)\n"
        "-o, --outdir\t\t\tDirectory in which to place output files\n"
        "OUTPUT_BASENAME\t\t\tFile prefix for output fastqs (should not conatin suffix (i.e.e .fastq)\n"
        "INPUT_FASTQ\t\t\t\tFastq file to split among the outputs. (-) signifies piping from stdin\n"
    );
}

// function definition for welcome message printer. This routine takes one
// arguments:
//      (1) uargs -> the userargs object with parsed content from the command line
// and has no return value, it simply prints a welcome message containing the
// passed arguments so that users can see in the logs the parameterization of the run
void print_welcomemsg(UserArgs uargs) {
    // print welcome message and the user specs:
    printf("\n  Welcome to fqsplit, the commandline utility for\n"
           "  Splitting FASTQ files for parallel processing. \n"
           "---------------------------------------------------\n"
           "User arguments:\n\n"
           " . Number Splits: %d\n"
           " . Number of Recs in round-robin file buffer: %d\n"
           " . Use Snakemake Scatter Format?: %d\n"
           " . Output Directory for Scattered Files: %s\n"
           " . Output File Basename for Scattered Files: %s\n"
           " . Input FASTQ File ( - for stdin): %s\n\n",
           uargs.nSplits, uargs.bufferRecs, uargs.smkFormat,
           uargs.outputDir, uargs.outputBase, uargs.inputFastq
    );
}

// function definition for print_userArgs: function takes a single 
// argument:
//      (1) uargs -> the userargs object to print
// returns:
//      void
void print_UserArgs(UserArgs uargs) {
    printf(
        "UserArgs(\n"
        " . n-splits: %i\n"
        " . Snakemake format?: %d\n"
        " . Output Directory: %s\n"
        " . Input FASTQ: %s\n"
        ")\n",
        uargs.nSplits, uargs.smkFormat,
        uargs.outputDir, uargs.inputFastq
    );
    return;
}

// function definition for flag_parser: the function takes five
// arguments:
//      (1) shortflag -> the shorthand string of the flag to parse
//      (2) longflag -> the longhand string of the flag to parse
//      (3) failstr -> the string to be returned if the parser fails
//      (4) argv -> the raw string array passed by the commandline
//      (5) argc -> the raw argument count (length of argv)
// returns:
//      (1) a copy of the string which accompanied the parsed flag,
//          or the failstr string, if the passed flag cannot be
//          found in the input arguments.
char * flag_parser(char *shortflag, char *longflag,
            char *failstr, char **argv, int argc) {
    int test; // if test == 0 we have match 
    for (int i=0; i < argc; i++) {
        test = 1; //initialize the value to no-match
        test = strcmp(argv[i], shortflag) * strcmp(argv[i], longflag);
        if ((test == 0 ) && (i < (argc - 1))) {
            return argv[i+1];
        }
    }
    return failstr;
}

// function definition for bool_parser: the function takes five
// arguments:
//      (1) shortflag -> the shorthand string of the flag to parse
//      (2) longflag -> the longhand string of the flag to parse
//      (3) uargs -> the userargs object to copy and return
//      (4) argv -> the raw string array passed by the commandline
//      (5) argc -> the raw argument count (length of argv)
// returns:
//      (1) true if the switch flag is present in the commandline input,
//          or false if the passed switch flag cannot be
//          found in the input arguments.
bool switch_parser(char *shortflag, char *longflag, char **argv, int argc) {
    int test; // if test == 0 we have match 
    for (int i=0; i < argc; i++) {
        test = 1; //initialize the value to no-match
        test = strcmp(argv[i], shortflag) * strcmp(argv[i], longflag);
        if (test == 0) {
            return true;
        }
    }
    return false;
}

/* File Operations Input/Output functions */

// function definition for file_open: the function, which initializes a
// passed FileInfo object. The FileInfo object needs to then be passed to the function
// file_close in order to close the file connetion and free the memory allocated
// to store the file path and mode. the function takes three arguments:
//      (1) fobj -> pointer to an declared FileInfo object
//      (2) fpath -> the file path of the file to open
//      (3) fmode -> the operation mode to open the file with (i.e. r, w, ..)
// returns:
//      (1) integer code, zero for success, 1 for failure if file cannot open
int file_open(FileInfo *fobj, char *fpath, char *fmode) {
    fobj->filePath = malloc(strlen(fpath)+1);
    fobj->fileMode = malloc(strlen(fmode)+1);
    strcpy(fobj->filePath, fpath);
    strcpy(fobj->fileMode, fmode);
    if (strcmp(fpath, "-") == 0) {
        fobj->fptr = stdin;
        return 0;
    } else {
        fobj->fptr = fopen(fobj->filePath, fobj->fileMode);
        if (fobj->fptr != NULL) {
            return 0;
        } else {
            return 1;
        }
    }
}

// function definition for file_close: the function, which has no return,
// closes the file connection inside the passed FileObject, and frees allocated
// memory for the filePath and fileMode variables. The function takes one
// argument:
//      (1) pointer to FileInfo object containing the the opened
//          file connection and file path, and file mode
// returns: no return value. This function is necessary to prevent memory
// leaks and issues with closing file connections
void file_close(FileInfo * fobj) {
    free(fobj->filePath);
    free(fobj->fileMode);
    fclose(fobj->fptr);
    return;
}

// function definition for build_outpath: the function which formats a 
// string representing the file path of the output fastq to be written
// the function takes as inputs, three arguments:
//      (1) pathstr -> a pointer to the string to hold the path
//      (2) uargs -> a copy of the user args structure containing passed params
//      (3) splitidx -> an integer representing the file number of the file to be
//          created. this builds the prefix which either is formatted using
//          smkFormat ({outputDir}/{splitidx}-of-{nSplits}.{outputBase}.fastq) or 
//          default   ({outputDir}/_{splitidx}.{outputBase}.fastq) 
// the function returns: no value
void build_outpath(char *pathstr, UserArgs uargs, int splitidx) {
    sprintf(pathstr, "%s/%d-of-%d.%s.fastq",
        uargs.outputDir, splitidx, uargs.nSplits, uargs.outputBase
    );
    return ;
}