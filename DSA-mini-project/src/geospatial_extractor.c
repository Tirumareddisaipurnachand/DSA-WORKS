#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_SIZE 16384
#define MAX_FIELDS 19
#define NAME_SIZE 256
#define COUNTRY_SIZE 10
#define CODE_SIZE 20


typedef struct
{
    long id;

    char name[NAME_SIZE];

    char asciiName[NAME_SIZE];

    double latitude;

    double longitude;

    char featureClass;

    char featureCode[CODE_SIZE];

    char country[COUNTRY_SIZE];

    char admin1[COUNTRY_SIZE];

    long population;

} Location;


/*
 * Check whether a GeoNames record is useful
 * for our project.
 */
int isUsefulLocation(const char *featureClass,
                     const char *featureCode)
{
    /*
     * Administrative locations
     *
     * Examples:
     * ADM1 = State / Province
     * ADM2 = District / County
     * ADM3 = Lower administrative division
     * ADM4 = Lower administrative division
     * ADM5 = Lower administrative division
     */
    if (featureClass[0] == 'A')
    {
        return 1;
    }


    /*
     * Political / populated locations
     */
    if (featureClass[0] == 'P')
    {
        /*
         * PCLI = Independent political entity
         */
        if (strncmp(featureCode, "PCLI", 4) == 0)
        {
            return 1;
        }


        /*
         * PPL = Populated place
         *
         * Examples:
         * City
         * Town
         * Village
         */
        if (strncmp(featureCode, "PPL", 3) == 0)
        {
            return 1;
        }
    }


    return 0;
}


/*
 * Split a GeoNames line using TAB characters.
 *
 * We do not use strtok() because empty fields
 * exist in the GeoNames dataset.
 */
int split_tab(char *line, char *fields[])
{
    int count = 0;

    char *start = line;


    for (char *p = line; *p != '\0'; p++)
    {
        /*
         * TAB found
         */
        if (*p == '\t')
        {
            *p = '\0';

            if (count < MAX_FIELDS)
            {
                fields[count++] = start;
            }

            start = p + 1;
        }


        /*
         * End of line
         */
        else if (*p == '\n' || *p == '\r')
        {
            *p = '\0';

            break;
        }
    }


    /*
     * Add final field
     */
    if (count < MAX_FIELDS)
    {
        fields[count++] = start;
    }


    return count;
}


int main()
{
    /*
     * Open the original GeoNames dataset.
     */
    FILE *inputFile = fopen(
        "data/allCountries.txt",
        "r"
    );


    if (inputFile == NULL)
    {
        printf(
            "Error: Could not open data/allCountries.txt\n"
        );

        return 1;
    }


    /*
     * Create the filtered location file.
     */
    FILE *outputFile = fopen(
        "data/locations.txt",
        "w"
    );


    if (outputFile == NULL)
    {
        printf(
            "Error: Could not create data/locations.txt\n"
        );

        fclose(inputFile);

        return 1;
    }


    char line[LINE_SIZE];

    char *fields[MAX_FIELDS];


    long totalLines = 0;

    long usefulLocations = 0;


    printf("\n");
    printf("========================================\n");
    printf("   GEOSPATIAL PLACE NAME EXTRACTOR\n");
    printf("========================================\n\n");

    printf("Reading GeoNames dataset...\n");
    printf("Creating data/locations.txt...\n\n");


    /*
     * Read every line from allCountries.txt.
     */
    while (fgets(
        line,
        sizeof(line),
        inputFile
    ) != NULL)
    {
        totalLines++;


        /*
         * Split line into fields.
         */
        int fieldCount = split_tab(
            line,
            fields
        );


        /*
         * Skip invalid records.
         */
        if (fieldCount < 19)
        {
            continue;
        }


        /*
         * Check whether this is a useful
         * geographic location.
         */
        if (!isUsefulLocation(
            fields[6],
            fields[7]
        ))
        {
            continue;
        }


        /*
         * Write the useful record into
         * locations.txt.
         *
         * Format:
         *
         * 1.  ID
         * 2.  Name
         * 3.  ASCII Name
         * 4.  Alternate Names
         * 5.  Latitude
         * 6.  Longitude
         * 7.  Feature Class
         * 8.  Feature Code
         * 9.  Country
         * 10. Admin1
         * 11. Population
         */
        fprintf(
            outputFile,
            "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",

            fields[0],   /* ID */

            fields[1],   /* Name */

            fields[2],   /* ASCII Name */

            fields[3],   /* Alternate Names */

            fields[4],   /* Latitude */

            fields[5],   /* Longitude */

            fields[6],   /* Feature Class */

            fields[7],   /* Feature Code */

            fields[8],   /* Country */

            fields[10],  /* Admin1 */

            fields[14]   /* Population */
        );


        usefulLocations++;


        /*
         * Show progress every 1 million records.
         */
        if (usefulLocations % 1000000 == 0)
        {
            printf(
                "Saved %ld useful locations...\n",
                usefulLocations
            );
        }
    }


    /*
     * Close both files.
     */
    fclose(inputFile);

    fclose(outputFile);


    /*
     * Display final information.
     */
    printf("\n");
    printf("========================================\n");

    printf(
        "Total lines read       : %ld\n",
        totalLines
    );

    printf(
        "Useful locations saved : %ld\n",
        usefulLocations
    );

    printf(
        "Output file            : data/locations.txt\n"
    );

    printf("========================================\n");


    return 0;
}