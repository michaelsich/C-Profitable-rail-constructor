//region LIBS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
//endregion

//region STRUCTS
/**
 * @brief This struct repr. a single legal railpart -
 * params are: left connector, right connector, part length and part price
 */
typedef struct RailPart
{
  char  leftConnection,
        rightConnection;
  long  length,
        price;
} RailPart;

/**
 * @brief This stuct saves the data of the main prices table for ease of passing info around the
 * program. params are: length (table length), width (table width) types: all possible connectors
 */
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
#define ERROR_IN_LINE       'e'         // case def for output -error in line
#define ERROR_IN_DIR        'i'         // case def for output -error in input dir
#define ERROR_EMPTY         'x'         // case def for output -error input empty
#define ERROR_USAGE         'u'         // case def for output -error in args
#define OUTPUT_CORRECT      'n'         // case def for output -success run
#define MIN_NATURAL_NUM     0           // minimal natural number to work with
#define BASE_FOR_NUMBERS    10          // Base to work with strtol
#define MIN_PART_LENGTH     0           // minimal length of single rail part
#define MIN_ALLOC_PARTS     20          // minimal allocated cells of RailPart in array
#define SPECIAL_ROWS        1           // additional rows in table (extra to desired len)

#define MAX_LEN_LINE        1025        // 1024 + /0
#define DEFAULT_OPTIMAL_VAL -1          // output val in case no optimal price for len
#define LINE_OF_DESIRED_LEN  1          // Line in input file of desired length
#define LINE_OF_TYPES_NUM    2          // Line in input file of number of connection types
#define LINE_OF_TYPES        3          // Line in input file of connection types


#define MSG_IN_LINE                     "Invalid input in line:"
#define MSG_NO_INPUT_FILE               "File doesn't exists."
#define MSG_USAGE                       "Usage: RailwayPlanner <InputFile>"
#define MSG_EMPTY_FILE                  "File is empty."
#define MSG_OPTIMAL_PRICE               "The minimal price is:"

const char      OUTPUT_DIR[100]      = "/home/michael/Desktop/railway_planner_output.txt";
// output file

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
void printTable(int** mainTable, PriceTableData* tableData);


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
char *readConnectionTypes(const long numOfChars, const char *input, PriceTableData *tableData);

void checkPartDetail(const char *types, char *input, int lineNum, PriceTableData *tableData);

int checkConnectionType(const char connection, const char *types);

void createRailPart(const char left, const char right, const long len, const long price);




int** allocPricesTable(PriceTableData* tableData);

void freeMainTable(int** mainTable, PriceTableData* tableData);

int canConnect(const char leftConnector, const char prevSuffix);

int isMatchingEnd(RailPart* part, const char currentConnector);

int isMatchingLen(RailPart* part, const long desiredLen);

void fillMinPrices(int** mainTable, PriceTableData* tableData);

int findMinPrice(int **mainTable,
                 PriceTableData *tableData,
                 const char suffix,
                 const int len);

int findSuffixCol(PriceTableData* tableData, int** table, char suffix);

int findOptimalPrice(const PriceTableData* tableData, int** table);

void freeForExit(void* ptrToFree);

void writeOutput(char oCase, const int appendix);
//endregion


int main(const int argc, const char *argv[])
{
    if (argc != 2)
    {
        writeOutput(ERROR_USAGE, 0); // note: 0 here has no meaning
        exit(EXIT_FAILURE);
    }
    isInputFileValid(argv[1]);


    int minPrice;
    gAllParts = (RailPart*)mallocAndCheck(MIN_ALLOC_PARTS * sizeof(RailPart));

    PriceTableData* tableData = readFileData(argv[1]);     // allocated pointer + 'types' allocated
    int** mainTable = allocPricesTable(tableData);          // allocated full dynamic table (2D)
    fillMinPrices(mainTable, tableData);

    //TODO: delete printers
//    printArr();
//    printf("-----\n");
//    printTable(mainTable, tableData);


    minPrice = findOptimalPrice(tableData, mainTable);
    freeMainTable(mainTable, tableData);
    writeOutput(OUTPUT_CORRECT, minPrice);
    return EXIT_SUCCESS;
}

//region MEMORY ALLOCTAION METHODS
//TODO: remember to mention a malloc'd ptr should be freed
void* mallocAndCheck(size_t allocSize)
{
    void *ptr = malloc(allocSize);
    if (ptr == NULL)
    {
        // forum instructor told we could ignore leaks in this case
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
        exit(EXIT_FAILURE);
    }
    return newPtr;
}
//endregion

//region TABLE HANDLER
int** allocPricesTable(PriceTableData* tableData)
{
    int ** mainTable = (int**)mallocAndCheck(sizeof(int*) * (tableData->length));

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
        freeForExit(mainTable[j]);
    }
    freeForExit(mainTable);

    //free tableData
    freeForExit(tableData->types);
    freeForExit(tableData);
}

void fillMinPrices(int** mainTable, PriceTableData* tableData)
{
    for (int i = SPECIAL_ROWS; i < tableData->length; ++i)
    {
        for (int j = 0; j < tableData->width; ++j)
        {
            mainTable[i][j] = findMinPrice(mainTable, tableData, (char)mainTable[0][j], i);
        }
    }
}

int findMinPrice(int **mainTable, PriceTableData *tableData, const char suffix, const int len)
{
    int     new_price       = INT_MAX,
            prevSuffixCol   = 0,
            prevSuffixRow   = 0;
    char    prevSuffix      ='\0';

    for (int p = 0; p < gIndexAllParts; ++p)  // p = part index (in parts array)
    {
        if (isMatchingEnd(&gAllParts[p], suffix)
            &&  isMatchingLen(&gAllParts[p], len)    )
        {
            prevSuffix      = gAllParts[p].leftConnection;
            prevSuffixCol   = findSuffixCol(tableData, mainTable, prevSuffix);
            prevSuffixRow   = len - (int)gAllParts[p].length;
            if (prevSuffixRow == 0)
                // reached zero length exactly
            {
                if (gAllParts[p].price < new_price)
                {
                    new_price = (int)gAllParts[p].price;
                }
            }
            else if ( canConnect(gAllParts[p].leftConnection,
                      mainTable[0][prevSuffixCol]) )
                // check left connector and prev suffix match
            {
                if (gAllParts[p].price + mainTable[prevSuffixRow][prevSuffixCol] < new_price)
                {
                    new_price = (int)gAllParts[p].price + mainTable[prevSuffixRow][prevSuffixCol];
                }
            }
        }
    }
    return new_price;
}

int canConnect(const char leftConnector, const char prevSuffix)
{
    if (leftConnector == prevSuffix)
    {
        return true;
    }
    return false;
}

int isMatchingEnd(RailPart* part, const char currentConnector)
{
    if (part->rightConnection == currentConnector)
    {
        return true;
    }
    return false;
}

int isMatchingLen(RailPart* part, const long desiredLen)
{
    if (part->length <= desiredLen)
    {
        return true;
    }
    return false;
}

int findSuffixCol(PriceTableData* tableData, int** table, char suffix)
{
    for (int j = 0; j < tableData->width; ++j)
    {
        if (table[0][j] == suffix)
        {
            return j;
        }
    }
    return -1;
}

int findOptimalPrice(const PriceTableData* tableData, int** table)
{
    int optimalPrice = INT_MAX;
    for (int i = 0; i < tableData->width; ++i)
    {
        if (table[tableData->length-1][i] < optimalPrice)
        {
            optimalPrice = table[tableData->length-1][i];
        }
    }
    if (optimalPrice == INT_MAX)
    {
        return DEFAULT_OPTIMAL_VAL;
    }
    return optimalPrice;
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

    char    line[MAX_LEN_LINE]="",
            part_input[MAX_LEN_LINE]="";
    char    *types = NULL;
    PriceTableData* tableData = (PriceTableData*)mallocAndCheck(sizeof(PriceTableData));

    FILE *file  = fopen(fileName, READ_ONLY);

    while (fgets(line, MAX_LEN_LINE, file))
    {
        char *strLeftover;

        iLine++;
        switch (iLine)
        {
            case 1: // Desired length
                desiredLen = strtol(line, &strLeftover, BASE_FOR_NUMBERS);
                if (!isInteger(desiredLen, MIN_NATURAL_NUM - 1, strLeftover))
                {
                    freeForExit(tableData);
                    writeOutput(ERROR_IN_LINE, LINE_OF_DESIRED_LEN);
                    fclose(file);
                    exit(EXIT_FAILURE);
                }
                tableData->length = (int) desiredLen + SPECIAL_ROWS;
                break;

            case 2:  // number of connection types
                amountConnectionTypes = strtol(line, &strLeftover, BASE_FOR_NUMBERS);
                if (!isInteger(amountConnectionTypes, MIN_NATURAL_NUM - 1, strLeftover))
                {
                    freeForExit(tableData);
                    writeOutput(ERROR_IN_LINE, LINE_OF_TYPES_NUM);
                    fclose(file);
                    exit(EXIT_FAILURE);
                }
                tableData->width = (int) amountConnectionTypes;
                break;

            case 3:  // connection types
                // allocated pointer
                types = readConnectionTypes(amountConnectionTypes, line, tableData);
                break;

            default:   // parts details
                strcpy(part_input, line);
                checkPartDetail(types, part_input, iLine, tableData);    // if valid adds part to
                // partsArr global
                break;
        }
    }
    fclose(file);
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

char *readConnectionTypes(const long numOfChars, const char *input, PriceTableData *tableData)
{
    //TODO: add mentioning to free pointer
    char *types = (char*)malloc(numOfChars + 1);
    if (types == NULL)
    {
        freeForExit(tableData);
        exit(EXIT_FAILURE);
    }

    if (numOfChars > 0)
    {
        int j = 0;
        for (int i = 0; i <(int)strlen(input)-1; ++i)
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
                freeForExit(types);
                freeForExit(tableData);
                writeOutput(ERROR_IN_LINE, LINE_OF_TYPES);
                exit(EXIT_FAILURE);
            }
            types[j++] = input[i];
        }
    }
    types[numOfChars] = '\0';
    return types;
}

void checkPartDetail(const char *types, char *input, int lineNum, PriceTableData *tableData)
{
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
                    printArr();
                    free((void*)types);
                    writeOutput(ERROR_IN_LINE, lineNum);
                    exit(EXIT_FAILURE);
                }
                break;

            case 1: // right connection type
                rightConnection = *token;
                if (!checkConnectionType(rightConnection, types))
                {
                    free((void*)types);
                    writeOutput(ERROR_IN_LINE, lineNum);
                    exit(EXIT_FAILURE);
                }
                break;
            case 2: // part length
                length = strtol(token, &leftovers, BASE_FOR_NUMBERS);
                if(!isInteger(length, MIN_PART_LENGTH, leftovers))
                {
                    free((void*)types);
                    writeOutput(ERROR_IN_LINE, lineNum);
                    exit(EXIT_FAILURE);
                }
                break;
            case 3: // part price
                price = strtol(token, &leftovers, BASE_FOR_NUMBERS);
                if (!isInteger(price, MIN_NATURAL_NUM - 1, leftovers))
                {
                    free((void*)types);
                    writeOutput(ERROR_IN_LINE, lineNum);
                    exit(EXIT_FAILURE);
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
    for (int i = 0; i <(int)strlen(types); ++i)
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
        // end of array - realloc
        gAllParts = (RailPart*)reallocAndCheck(gAllParts,
                                               (gPartsArrCapacity + MIN_ALLOC_PARTS) * sizeof
                                                       (RailPart));
        gPartsArrCapacity += MIN_ALLOC_PARTS;
    }

    gAllParts[gIndexAllParts++] = newRail;
}
//endregion

void freeForExit(void* ptrToFree)
{
    if (ptrToFree)
    {
        free(ptrToFree);
        ptrToFree = NULL;
    }
    if (gAllParts)
    {
        free(gAllParts);
        gAllParts = NULL;
    }
}

void writeOutput(char oCase, const int appendix)
{
    char out [MAX_LEN_LINE];
    //TODO: add checkign fopen success
    FILE *outputFile = fopen(OUTPUT_DIR, WRITE);
    //TODO: move up:
    int DEF_LEN = MAX_LEN_LINE;
    switch (oCase)
    {
        case ERROR_USAGE:
            strcpy(out, MSG_USAGE);
            break;
        case ERROR_IN_DIR:
            strcpy(out, MSG_NO_INPUT_FILE);
            break;
        case ERROR_EMPTY:
            strcpy(out, MSG_EMPTY_FILE);
            break;
        case ERROR_IN_LINE:
            snprintf(out, DEF_LEN, MSG_IN_LINE" %d." , appendix);
            break;
        case OUTPUT_CORRECT:
            snprintf(out, DEF_LEN, MSG_OPTIMAL_PRICE" %d" , appendix);
            break;
        default:
            break;
    }
    //TODO: delete printf
    printf("%s", out);
    fputs(out, outputFile);
    fclose(outputFile);
}

//region DEBUGGING
//TODO: delete debug method
void printArr()
{
    for (int i = 0; i < gIndexAllParts; ++i)
    {
        printf("%d: %c%c %ld-%ld\n", i,
                gAllParts[i].leftConnection,
                gAllParts[i].rightConnection,
                gAllParts[i].length,
                gAllParts[i].price
        );
    }
    printf("\n");
}

void printTable(int** mainTable, PriceTableData* tableData)
{
    printf("\t  width\n   ");
    for (int c = 0; c < tableData->width; ++c )
    {
        printf("  %c|", mainTable[0][c]);
    }
    printf("\n");
    for (int i = 1; i < tableData->length; ++i)
    {
        printf("%d| ", i);
        for (int j = 0; j < tableData->width; ++j)
        {
            if (mainTable[i][j] && mainTable[i][j] != INT_MAX)
                printf("%d| ", mainTable[i][j]);
            else
                printf("XX| ");
        }
        printf("\n");
    }
}
//endregion