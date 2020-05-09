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
const char*     PART_DELIMITER       = ","; // for strtok function
// output file

//endregion

//region GLOBALS
RailPart*   gAllParts;                              // pointer to pointer of dynamic array of parts
int         gIndexAllParts = 0;                     // index of first free cell in parts array
int         gPartsArrCapacity = MIN_ALLOC_PARTS;    // current capacity of parts array
//endregion

//region FUNCTIONS DECLARATION
/**
 * @brief alloc's the received size and returns a pointer to it. checks alloc success
 * @param allocSize size to alloc
 * @return alloc'd pointer NOTE! THIS POINTER SHOULD BE FREED BY CALLER
 */
void* mallocAndCheck(size_t allocSize);
/**
 * @brief relloc's the received size and  pointer. checks realloc success
 * @param ptr pointer to current allocated memory
 * @param newSize size to alloc
 * @return alloc'd pointer NOTE! THIS POINTER SHOULD BE FREED BY CALLER
 */
void* reallocAndCheck(void* ptr, size_t newSize);

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
/**
 * @brief parses, checks and handles the input file data
 * @param fileName input file to be analyzed
 * @return pointer to a struct containing the prices tables data (see struct doc)
 *          NOTE: this pointer is alloc`d and should be freed by caller
 */
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
 * @param testedNum the number to test (type long, result of strtol)
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
 * @param tableData current information on prices table, passed to free memory in case of error
 * @param file input file - passed in case of error found to close file
 * @return ptr to array of chars with all valid symbols (allocated)
 */
char * readConnectionTypes(const long numOfChars, const char *input,
                           PriceTableData *tableData,
                           FILE *file);
/**
 * @brief checks if single part information is valid, if valid adds details to gPartsArr
 * @param input current line of input (to be analyzed)
 * @param tableData table information containing valid connectors information
 * @return true - if adding part to array succeded, false otherwise
 */
int checkPartDetail(char *input, PriceTableData *tableData);
/**
 * @brief checks that received connection appears in given connector types
 * @param connection char repr. the tested connector
 * @param types char array containing al valid possible connectors
 * @return true if valid, false otherwise
 */
int checkConnectionType(const char connection, const char *types);
/**
 * @brief creates an instance of RailPart and adds it to gPartsArr
 * @param left valid left connector of part
 * @param right valid right connector of part
 * @param len valid part length
 * @param price valid price of part
 */
void createRailPart(const char left, const char right, const long len, const long price);


/**
 * @brief allocates all needed memory to create prices table 2d dynamic array
 * @param tableData struct containing the size and types of the table
 * @return pointer to int pointer array - 2d array repr. prices
 */
int** allocPricesTable(PriceTableData* tableData);
/**
 * @brief frees all allocated memory of the prices table
 * @param mainTable pointer to 2d array repr. prices table to be freed
 * @param tableData  pointer to struct containing the talbe's info
 */
void freeMainTable(int** mainTable, PriceTableData* tableData);
/**
 * @brief verfies current part's left connector matches previous parts connector
 * @param leftConnector current part's left connector
 * @param prevSuffix right connector of previous parts
 * @return true - if connection possible, false otherwise
 */
int canConnect(const char leftConnector, const char prevSuffix);
/**
 * @brief checks if the current part's right connector matches desired connector accord. to table
 * @param part current tested part pointer
 * @param currentConnector desired connector (column in table)
 * @return true if match, false otherwise
 */
int isMatchingEnd(RailPart* part, const char currentConnector);
/**
 * @brief checks that the current part's length matches the current length in table
 * @param part current tested part
 * @param desiredLen desired length (row in table)
 * @return true if diff between part's length and desired length is non-negative, false otherwise
 */
int isMatchingLen(RailPart* part, const long desiredLen);
/**
 * @brief main method for filling prices into prices table, checks all conditions
 * @param mainTable pointer to 2d array repr. main prices table
 * @param tableData pointer to struct containing talbe info
 */
void fillMinPrices(int** mainTable, PriceTableData* tableData);
/**
 * @brief runs on all parts and finds minimal price for each cell in table
 * @param mainTable pointer to main prices table
 * @param tableData pointer to tables information (size types etc..)
 * @param suffix current desired right connector
 * @param len current desired length
 * @return optimal price for current table cell, INT_MAX if none exists
 */
int findMinPrice(int **mainTable,
                 PriceTableData *tableData,
                 const char suffix,
                 const int len);
/**
 * @brief finds column of desired connector suffix
 * @param tableData struct describing table's information
 * @param table pointer to 2d array repr. main prices talbe
 * @param suffix desired connector
 * @return column number of desired connector in table
 */
int findSuffixCol(PriceTableData* tableData, int** table, char suffix);
/**
 * @brief finds the optimal price for inputed desired length
 * @param table pointer to main prices table
 * @param tableData pointer to tables information (size types etc..)
 * @return optimal price for largest length in table, INT_MAX if none exists
 */
int findOptimalPrice(const PriceTableData* tableData, int** table);
/**
 * @brief frees given pointer and sets its value to NULL, frees global allocted pointers
 * @param ptrToFree
 */
void freeForExit(void* ptrToFree);
/**
 * @brief frees all given pointers and exit program with FAILURE CODE
 * @param tableData pointer to struct with info about the prices talbe
 * @param file current open file
 * @param types pointer to alloc'd string
 */
void freeTypesAndExit(PriceTableData* tableData, FILE* file, char* types);
/**
 * @brief creates and writes the output file
 * @param oCase char repr. the exit case (like ENUMS)
 * @param appendix line or price to print. should be set to 0 if not needed
 */
void writeOutput(char oCase, const int appendix);
//endregion

/**
 * @brief entry point for RailWAyPlanner program - find optimal size for given length
 * @param argc number of arguments
 * @param argv arguments array
 * @return 1 on failure, 0 on success
 */
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

    minPrice = findOptimalPrice(tableData, mainTable);
    freeMainTable(mainTable, tableData);
    writeOutput(OUTPUT_CORRECT, minPrice);
    return EXIT_SUCCESS;
}

//region MEMORY ALLOCTAION METHODS
/**
 * @brief alloc's the received size and returns a pointer to it. checks alloc success
 * @param allocSize size to alloc
 * @return alloc'd pointer NOTE! THIS POINTER SHOULD BE FREED BY CALLER
 */
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
/**
 * @brief relloc's the received size and  pointer. checks realloc success
 * @param ptr pointer to current allocated memory
 * @param newSize size to alloc
 * @return alloc'd pointer NOTE! THIS POINTER SHOULD BE FREED BY CALLER
 */
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
/**
 * @brief allocates all needed memory to create prices table 2d dynamic array
 * @param tableData struct containing the size and types of the table
 * @return pointer to int pointer array - 2d array repr. prices
 */
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

/**
 * @brief frees all allocated memory of the prices table
 * @param mainTable pointer to 2d array repr. prices table to be freed
 * @param tableData  pointer to struct containing the talbe's info
 */
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

/**
 * @brief main method for filling prices into prices table, checks all conditions
 * @param mainTable pointer to 2d array repr. main prices table
 * @param tableData pointer to struct containing talbe info
 */
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

/**
 * @brief runs on all parts and finds minimal price for each cell in table
 * @param mainTable pointer to main prices table
 * @param tableData pointer to tables information (size types etc..)
 * @param suffix current desired right connector
 * @param len current desired length
 * @return optimal price for current table cell, INT_MAX if none exists
 */
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

/**
 * @brief verfies current part's left connector matches previous parts connector
 * @param leftConnector current part's left connector
 * @param prevSuffix right connector of previous parts
 * @return true - if connection possible, false otherwise
 */
int canConnect(const char leftConnector, const char prevSuffix)
{
    if (leftConnector == prevSuffix)
    {
        return true;
    }
    return false;
}

/**
 * @brief checks if the current part's right connector matches desired connector accord. to table
 * @param part current tested part pointer
 * @param currentConnector desired connector (column in table)
 * @return true if match, false otherwise
 */
int isMatchingEnd(RailPart* part, const char currentConnector)
{
    if (part->rightConnection == currentConnector)
    {
        return true;
    }
    return false;
}

/**
 * @brief checks that the current part's length matches the current length in table
 * @param part current tested part
 * @param desiredLen desired length (row in table)
 * @return true if diff between part's length and desired length is non-negative, false otherwise
 */
int isMatchingLen(RailPart* part, const long desiredLen)
{
    if (part->length <= desiredLen)
    {
        return true;
    }
    return false;
}

/**
 * @brief finds column of desired connector suffix
 * @param tableData struct describing table's information
 * @param table pointer to 2d array repr. main prices talbe
 * @param suffix desired connector
 * @return column number of desired connector in table
 */
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

/**
 * @brief finds the optimal price for inputed desired length
 * @param table pointer to main prices table
 * @param tableData pointer to tables information (size types etc..)
 * @return optimal price for largest length in table, INT_MAX if none exists
 */
int findOptimalPrice(const PriceTableData* tableData, int** table)
{
    int optimalPrice = INT_MAX;
    if (tableData->length == MIN_NATURAL_NUM + 1)  // desired length is zero
    {
        return 0;
    }
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
/**
 * @brief runs sanity checks on file (existence, empty)
 * @param fileName (string) full file address
 */
void isInputFileValid(const char *fileName)
{
    if (!isFileExist(fileName))
    {
        writeOutput(ERROR_IN_DIR, 0); // note: 0 here has no meaning
        exit(EXIT_FAILURE);
    }
    if (isFileEmpty(fileName))
    {
        writeOutput(ERROR_EMPTY, 0); // note: 0 here has no meaning
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief checks if a given file address exist
 * @param fileName (string) full file address
 * @return true if exists, false otherwise
 */
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

/**
 * @brief checks if a given file is empty
 * @param fileName (string) full file address
 * @return true if empty, false otherwise
 */
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

/**
 * @brief parses, checks and handles the input file data
 * @param fileName input file to be analyzed
 * @return pointer to a struct containing the prices tables data (see struct doc)
 *          NOTE: this pointer is alloc`d and should be freed by caller
 */
PriceTableData* readFileData(const char *fileName)
{
    int     iLine                   = 0;
    long    desiredLen              = 0,
            amountConnectionTypes   = 0;

    char    line[MAX_LEN_LINE]       = "",
            part_input[MAX_LEN_LINE] = "";
    char*   types                   = NULL;
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
                if (!isInteger(amountConnectionTypes, MIN_NATURAL_NUM , strLeftover))
                {
                    freeForExit(tableData);
                    writeOutput(ERROR_IN_LINE, LINE_OF_TYPES_NUM);
                    fclose(file);
                    exit(EXIT_FAILURE);
                }
                tableData->width = (int) amountConnectionTypes;
                break;

            case 3:  // connection types
                // NOTE: allocated pointer
                types = readConnectionTypes(amountConnectionTypes, line, tableData, file);
                tableData->types = types;
                break;

            default:   // parts details
                strcpy(part_input, line);
                if (!checkPartDetail(part_input, tableData))
                {
                    writeOutput(ERROR_IN_LINE, iLine);
                    freeForExit(types);
                    freeForExit(tableData);
                    fclose(file);
                    exit(EXIT_FAILURE);

                }
                break;
        }
    }
    fclose(file);
    return tableData;
}

/**
 * @brief checks that testedNum is greater than referenceNum
 * @param testedNum the tested number
 * @param referenceNum a reference number to check testedNum is greater than him
 * @return True if greater, false otherwise
 */
int compareToRef(const long testedNum, const int referenceNum)
{
    if (testedNum > referenceNum)
    {
        return true;
    }
    return false;
}

/**
 * @brief checks that testedNum is an int greater than refNum
 * @param testedNum the number to test (type long, result of strtol)
 * @param refNum a reference number to check testedNum is greater than him
 * @param leftovers ptr tp char array containing the remains of the string of strtol
 * @return true if int and greater than refNum, false otherwise
 */
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

/**
 * @brief analyzes the 3rd line of the input file - checks validity of connections types chars
 * NOTE: this function returns an allocated pointer that should be freed when served its purpose
 * @param numOfChars the number of chars to be found (line 2 of input file)
 * @param input ptr to char array containing the 3rd line of the input
 * @param tableData current information on prices table, passed to free memory in case of error
 * @param file input file - passed in case of error found to close file
 * @return ptr to array of chars with all valid symbols (allocated)
 */
char* readConnectionTypes(const long numOfChars, const char *input,
                           PriceTableData *tableData,
                           FILE *file)
{
    //TODO: add mentioning to free pointer
    char *types = (char*)malloc(numOfChars + 1);
    if (types == NULL)
    {
        freeForExit(tableData);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    if (numOfChars > 0)
    {
        int j = 0;
        for (int i = 0; i < (int)strlen(input); ++i)
        {
            if (  (i % 2 != 0)  && ( (input[i] == DELIMITER) || (input[i] == (char)'\n') )  )
            {   // enters if index is odd and char is delimiter or end of line (valid)
                continue;
            }
            else if ( (i % 2 != 0) )
            {       // if unexpected delimiter (invalid)
                freeTypesAndExit(tableData, file, types);
            }
            else if ( (i % 2 == 0) && (( (input[i] == DELIMITER) || (input[i] == (char)'\n') )) )
            {   // enters if even index (connector expected) but got delimiter instead
                freeTypesAndExit(tableData, file, types);
            }
            types[j++] = input[i];
        }
    }
    types[numOfChars] = '\0';
    return types;
}


/**
 * @brief checks if single part information is valid, if valid adds details to gPartsArr
 * @param input current line of input (to be analyzed)
 * @param tableData table information containing valid connectors information
 * @return true - if adding part to array succeded, false otherwise
 */
int checkPartDetail(char *input, PriceTableData *tableData)
{
    int     i           = 0;
    long    length      = 0,
            price       = 0;
    char *leftovers;
    char  leftConnection, rightConnection;

    char* token = strtok(input, PART_DELIMITER);
    while (token != NULL)
    {
        switch (i)
        {
            case 0: // left connection type
                leftConnection = *token;
                if (strlen(token) != 1 || (!checkConnectionType(leftConnection, tableData->types)))
                {   // enters if found more than 1 char or char invalid (not in types)
                    return false;
                }
                break;

            case 1: // right connection type
                rightConnection = *token;
                if (strlen(token) != 1 || (!checkConnectionType(rightConnection, tableData->types)))
                {   // enters if found more than 1 char or char invalid (not in types)
                    return false;
                }
                break;

            case 2: // part length
                length = strtol(token, &leftovers, BASE_FOR_NUMBERS);
                if(!isInteger(length, MIN_PART_LENGTH, leftovers))
                {
                    return false;
                }
                break;

            case 3: // part price
                price = strtol(token, &leftovers, BASE_FOR_NUMBERS);
                if (!isInteger(price, MIN_NATURAL_NUM, leftovers))
                {
                    return false;
                }
                break;
            default:
                break;
        }
        token = strtok(NULL, PART_DELIMITER);
        i++;
    }
    createRailPart(leftConnection, rightConnection, length, price);
    return true;
}


/**
 * @brief checks that received connection appears in given connector types
 * @param connection char repr. the tested connector
 * @param types char array containing al valid possible connectors
 * @return true if valid, false otherwise
 */
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

/**
 * @brief creates an instance of RailPart and adds it to gPartsArr
 * @param left valid left connector of part
 * @param right valid right connector of part
 * @param len valid part length
 * @param price valid price of part
 */
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


//region FUNCTIONS: FREEiers and OUTPUT
/**
 * @brief frees given pointer and sets its value to NULL, frees global allocted pointers
 * @param ptrToFree
 */
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

/**
 * @brief frees all given pointers and exit program with FAILURE CODE
 * @param tableData pointer to struct with info about the prices talbe
 * @param file current open file
 * @param types pointer to alloc'd string
 */
void freeTypesAndExit(PriceTableData* tableData, FILE* file, char* types)
{
    freeForExit(types);
    freeForExit(tableData);
    fclose(file);
    writeOutput(ERROR_IN_LINE, LINE_OF_TYPES);
    exit(EXIT_FAILURE);
}

/**
 * @brief creates and writes the output file
 * @param oCase char repr. the exit case (like ENUMS)
 * @param appendix line or price to print. should be set to 0 if not needed
 */
void writeOutput(char oCase, const int appendix)
{
    char out [MAX_LEN_LINE];
    FILE *outputFile = fopen(OUTPUT_DIR, WRITE);
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
            snprintf(out, MAX_LEN_LINE, MSG_IN_LINE" %d." , appendix);
            break;
        case OUTPUT_CORRECT:
            snprintf(out, MAX_LEN_LINE, MSG_OPTIMAL_PRICE" %d" , appendix);
            break;
        default:
            break;
    }

    fputs(out, outputFile);
    fclose(outputFile);
}
//endregion
