//region LIBS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
//endregion

//region STRUCTS
//TODO: add description
typedef struct RailPart
{
  char  leftConnection,
        rightConnection;
  long  length,
        price;
} RailPart;

typedef struct PriceTableData
{
    int     length;     // repr. all possible length +1 types row
    int     width;      // repr. all possible connection types endings
    char*   types;      // all possible connections as chars
} PriceTableData;
//endregion

//region CONSTANTS
#define WRITE               "w+"        // writing to file flag
#define READ_ONLY           "r"         // reading from file flag
#define DELIMITER           ','         // delimiter of input file content
#define MIN_NATURAL_NUM     0           // minimal natural number to work with
#define BASE_FOR_NUMBERS    10          // Base to work with strtol
#define MIN_PART_LENGTH     1           // minimal length of single rail part
#define MIN_ALLOC_PARTS     15          // minimal allocated cells of RailPart in array

const int   MAX_LEN_LINE                = 1025; // 1024 + /0
//endregion

//region GLOBALS
RailPart*   gAllParts;                              // pointer to pointer of dynamic array of parts
int         gIndexAllParts = 0;                     // index of first free cell in parts array
int         gPartsArrCapacity = MIN_ALLOC_PARTS;    // current capacity of parts array
//endregion

//region FUNCTIONS DECLARATION
void* mallocAndCheck(size_t allocSize);

void* reallocAndCheck(void* ptr, size_t newSize);

//TODO: delete debug method
void printArr();

/**
 * @brief runs sanity checks on file (existence, empty)
 * @param fileName (string) full file address
 */
void isInputFileValid(const char *fileName);
/**
 * @brief checks if a given file address exist
 * @param fileName (string) full file address
 * @return true if exists, false otherwise
 */
int isFileExist(const char *fileName);
/**
 * @brief checks if a given file is empty
 * @param fileName (string) full file address
 * @return true if empty, false otherwise
 */
int isFileEmpty(const char *fileName);

PriceTableData* readFileData(const char *fileName);

/**
 * @brief checks that testedNum is greater than referenceNum
 * @param testedNum the tested number
 * @param referenceNum a reference number to check testedNum is greater than him
 * @return True if greater, false otherwise
 */
int compareToRef(const long testedNum, const int referenceNum);
/**
 * @brief checks that testedNum is an int greater than refNum
 * @param testedNum the number to test (long, result of strtol)
 * @param refNum a reference number to check testedNum is greater than him
 * @param leftovers ptr tp char array containing the remains of the string of strtol
 * @return true if int and greater than refNum, false otherwise
 */
int isInteger(const long testedNum, const int refNum, const char* leftovers);
/**
 * @brief analyzes the 3rd line of the input file - checks validity of connections types chars
 * NOTE: this function returns an allocated pointer that should be freed when served its purpose
 * @param numOfChars the number of chars to be found (line 2 of input file)
 * @param input ptr to char array containing the 3rd line of the input
 * @return ptr to array of chars with all valid symbols
 */
char* readConnectionTypes(const long numOfChars, const char* input);

int checkPartDetail(const char *types, char *input);

int checkConnectionType(const char connection, const char *types);

void createRailPart(const char left, const char right, const long len, const long price);

int** allocPricesTable(PriceTableData* tableData);

void freeMainTable(int** mainTable, PriceTableData* tableData);

//endregion


int main(int argc, char *argv[])
{
//    if (argc != 2)
//    {
//        //TODO: add to output "Usage: RailWayPlanner <inputFile>"
//        exit(EXIT_FAILURE);
//    }
//    isInputFileValid(argv[1]);

    //TODO: delete this:
    char file_adr[70] = "/home/michael/CLionProjects/C_ex2/in.txt";
    gAllParts = (RailPart*)mallocAndCheck(MIN_ALLOC_PARTS * sizeof(RailPart));

    PriceTableData* tableData = readFileData(file_adr);     // allocated pointer + 'types' allocated
    int** mainTable = allocPricesTable(tableData);         // allocated full dynamic table (2D)

    freeMainTable(mainTable, tableData);
    return EXIT_SUCCESS;
}


//region MEMORY ALLOCTAION METHODS
//TODO: remember to mention a malloc'd ptr should be freed
void* mallocAndCheck(size_t allocSize)
{
    void *ptr = malloc(allocSize);
    if (ptr == NULL)
    {
        //TODO: add output malloc fail
        exit(EXIT_FAILURE);
    }
    return ptr;
}

//TODO: remember to mention a malloc'd ptr should be freed
void* reallocAndCheck(void* ptr, size_t newSize)
{
    void *newPtr = realloc(ptr, newSize);
    if (newPtr == NULL)
    {
        free(ptr);
        //TODO: add output malloc fail
        exit(EXIT_FAILURE);
    }
    return newPtr;
}
//endregion

//region TABLE HANDLER
int** allocPricesTable(PriceTableData* tableData)
{
    int ** mainTable = (int**)mallocAndCheck(sizeof(int*) * (tableData->length + 1));

    for (int i = 0; i < tableData->length; ++i)
    {
        mainTable[i] = (int*)mallocAndCheck(sizeof(int) * tableData->width);
    }
    for (int j = 0; j < tableData->width; ++j)
    {
        mainTable[0][j] = tableData->types[j];
    }
    return mainTable;
}

void freeMainTable(int** mainTable, PriceTableData* tableData)
{
    for (int j = 0; j < tableData->length; ++j)
    {
        free(mainTable[j]);
        mainTable[j] = NULL;
    }
    free(mainTable);
    mainTable = NULL;

    //free tableData
    free(tableData->types);
    tableData->types = NULL;
    free(tableData);
    tableData = NULL;
}
//endregion

//region Functions: INPUT HANDLING
void isInputFileValid(const char *fileName)
{
    if (!isFileExist(fileName))
    {
        //TODO: add to output "File doesn't exists"
        exit(EXIT_FAILURE);
    }
    if (isFileEmpty(fileName))
    {
        //TODO: add to output "File is empty"
        exit(EXIT_FAILURE);
    }
}

int isFileExist(const char *fileName)
{
    FILE *file;
    if ((file = fopen(fileName, READ_ONLY)))
    {
        fclose(file);
        return true;
    }
    return false;
}

int isFileEmpty(const char *fileName)
{
    FILE *file = fopen(fileName, READ_ONLY);
    int ch = fgetc(file);
    if (ch == EOF)
    {
        fclose(file);
        return true;
    }
    ungetc(ch, file);
    fclose(file);
    return false;
}

PriceTableData* readFileData(const char *fileName)
{
    int     iLine                   = 0;
    long    desiredLen              = 0,
            amountConnectionTypes   = 0;

    char    line[MAX_LEN_LINE],
            part_input[MAX_LEN_LINE];
    char    *types;
    PriceTableData* tableData = (PriceTableData*)mallocAndCheck(sizeof(PriceTableData));

    FILE *file  = fopen(fileName, READ_ONLY);

    while (fgets(line, MAX_LEN_LINE, file))
    {
        char *strLeftover;
        char input[amountConnectionTypes * 2];

        iLine++;
        switch (iLine)
        {
            case 1: // Desired length
                desiredLen = strtol(line, &strLeftover, BASE_FOR_NUMBERS);
                if (!isInteger(desiredLen, MIN_NATURAL_NUM - 1, strLeftover))
                {
                    //TODO: add error and line number to output
                    return false;
                }
                tableData->length = desiredLen + 1;
                break;

            case 2:  // number of connection types
                amountConnectionTypes = strtol(line, &strLeftover, BASE_FOR_NUMBERS);
                if (!isInteger(amountConnectionTypes, MIN_NATURAL_NUM - 1, strLeftover))
                {
                    //TODO: add error and line number to output
                    return false;
                }
                tableData->width = amountConnectionTypes;
                break;

            case 3:  // connection types
                strcpy(input, line);
                types = readConnectionTypes(amountConnectionTypes, input);    //allocated pointer
                break;

            default:   // parts details
                strcpy(part_input, line);
                checkPartDetail(types, part_input);    // if valid adds part to partsArr global
                break;
        }
    }
    tableData->types = types;
    return tableData;
}

int compareToRef(const long testedNum, const int referenceNum)
{
    if (testedNum > referenceNum)
    {
        return true;
    }
    return false;
}

int isInteger(const long testedNum, const int refNum, const char* leftovers)
{
    if (leftovers[0] == (char)'\n' || leftovers[0] == 0)         // check that integer
    {
        if (compareToRef(testedNum, refNum))  // non-negative number
        {
            // gets here if positive int
            return true;
        }
    }
    return false;
}

char* readConnectionTypes(const long numOfChars, const char* input)
{
    //TODO: add mentioning to free pointer
    char *types = (char*)malloc(numOfChars+1);
    if (types == NULL)
    {
        // TODO: add exit error to output
        exit(EXIT_FAILURE);
    }

    if (numOfChars > 0)
    {
        int j = 0;
        for (int i = 0; i < strlen(input)-1; ++i)
        {
            if (i % 2 != 0)
            {
                if ( input[i] != DELIMITER)
                {
                    // gets here if odd position is not delimiter
                    free(types);
                    // TODO: add error to output "in line 3"
                    exit(EXIT_FAILURE);
                }
                continue;
            }
            else if ( (input[i] == (char)'\n') || (input[i] == (char)DELIMITER) )
            {
                // if even index and not a valid char
                free(types);
                // TODO: add error to output "in line 3"
                exit(EXIT_FAILURE);
            }
            types[j++] = input[i];
        }
    }
    types[numOfChars] = '\0';
    return types;
}

int checkPartDetail(const char *types, char *input)
{
    //TODO: pass line number to here to throw on error
    int     i           = 0;
    long    length      = 0,
            price       = 0;
    const char* delimiter = ",";
    char *leftovers;
    char  leftConnection, rightConnection;

    char* token = strtok(input, delimiter);
    while (token != NULL)
    {
        switch (i)
        {
            case 0: // left connection type
                leftConnection = *token;
                if (!checkConnectionType(leftConnection, types))
                {
                    free((void*)types);
                    // TODO: add Error
                }
                break;

            case 1: // right connection type
                rightConnection = *token;
                if (!checkConnectionType(rightConnection, types))
                {
                    free((void*)types);
                    // TODO: add Error
                }
                break;
            case 2: // part length
                length = strtol(token, &leftovers, BASE_FOR_NUMBERS);
                if(isInteger(length, MIN_PART_LENGTH, leftovers))
                break;
            case 3: // part price
                price = strtol(token, &leftovers, BASE_FOR_NUMBERS);
                if (!isInteger(price, MIN_NATURAL_NUM - 1, leftovers))
                {
                    free((void*)types);
                    // TODO: add Error
                }
                break;
            default:
                break;
        }
        token = strtok(NULL, delimiter);
        i++;
    }
    createRailPart(leftConnection, rightConnection, length, price);
}

int checkConnectionType(const char connection, const char *types)
{
    for (int i = 0; i < strlen(types); ++i)
    {
        if (connection == types[i])
        {
            return true;
        }
    }
    return false;
}

void createRailPart(const char left, const char right, const long len, const long price)
{
    RailPart newRail;
    newRail.leftConnection      = left;
    newRail.rightConnection     = right;
    newRail.price               = price;
    newRail.length              = len;

    if (gPartsArrCapacity == gIndexAllParts)
    {
        // TODO: URGENT!! figure out why crashing if realloacting
        // end of array - realloc
//        gAllParts = reallocAndCheck(gAllParts, gPartsArrCapacity + MIN_ALLOC_PARTS);
        gAllParts = (RailPart*)realloc(gAllParts, gPartsArrCapacity+MIN_ALLOC_PARTS);
        gPartsArrCapacity += MIN_ALLOC_PARTS;
    }

    gAllParts[gIndexAllParts++] = newRail;
}
//endregion


//region DEBUGGING
//TODO: delete debug method
void printArr()
{
    for (int i = 0; i < gIndexAllParts; ++i)
    {
        printf("%d: %c%c %ld-%ld\n",i,
                gAllParts[i].leftConnection,
                gAllParts[i].rightConnection,
                gAllParts[i].length,
                gAllParts[i].price
        );
    }
    printf("\n");
}
//endregion