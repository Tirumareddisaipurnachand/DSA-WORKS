#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_SIZE 16384
#define MAX_FIELDS 19
#define NAME_SIZE 256
#define MAX_LOCATIONS 6000000

typedef struct
{
    char name[NAME_SIZE];
    char featureCode[20];
    long population;
} Location;


/*
 * Convert uppercase English characters
 * to lowercase.
 */
char toLowerChar(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c + ('a' - 'A');

    return c;
}


/*
 * Case-insensitive comparison.
 */
int caseInsensitiveCompare(
    const char *a,
    const char *b
)
{
    while (*a && *b)
    {
        char ca = toLowerChar(*a);
        char cb = toLowerChar(*b);

        if (ca != cb)
            return 0;

        a++;
        b++;
    }

    return *a == '\0' && *b == '\0';
}


/*
 * Split GeoNames TAB-separated line.
 *
 * Empty fields are preserved.
 */
int split_tab(
    char *line,
    char *fields[]
)
{
    int count = 0;

    char *start = line;

    for (
        char *p = line;
        *p != '\0';
        p++
    )
    {
        if (*p == '\t')
        {
            *p = '\0';

            if (count < MAX_FIELDS)
                fields[count++] = start;

            start = p + 1;
        }

        else if (
            *p == '\n' ||
            *p == '\r'
        )
        {
            *p = '\0';
            break;
        }
    }

    if (count < MAX_FIELDS)
        fields[count++] = start;

    return count;
}


/*
 * Determine whether the location is useful.
 *
 * A = administrative locations
 * P = populated places
 */
int isUsefulLocation(
    const char *featureClass,
    const char *featureCode
)
{
    if (featureClass[0] == 'A')
        return 1;

    if (featureClass[0] == 'P')
    {
        if (strncmp(
                featureCode,
                "PCLI",
                4
            ) == 0)
        {
            return 1;
        }

        if (strncmp(
                featureCode,
                "PPL",
                3
            ) == 0)
        {
            return 1;
        }
    }

    return 0;
}


/*
 * Main function.
 */
int main()
{
    FILE *file =
        fopen(
            "data/locations.txt",
            "r"
        );

    if (file == NULL)
    {
        printf(
            "Error: Could not open "
            "data/locations.txt\n"
        );

        return 1;
    }


    /*
     * Allocate memory for locations.
     */
    Location *locations =
        malloc(
            MAX_LOCATIONS *
            sizeof(Location)
        );

    if (locations == NULL)
    {
        printf(
            "Error: Memory allocation failed.\n"
        );

        fclose(file);

        return 1;
    }


    char line[LINE_SIZE];

    char *fields[MAX_FIELDS];

    long locationCount = 0;


    printf(
        "========================================\n"
    );

    printf(
        "        LOCATION INDEX BUILDER\n"
    );

    printf(
        "========================================\n\n"
    );


    /*
     * Read locations.txt.
     */
    while (
        fgets(
            line,
            sizeof(line),
            file
        ) != NULL
    )
    {
        int fieldCount =
            split_tab(
                line,
                fields
            );


        if (fieldCount < 11)
            continue;


        /*
         * locations.txt:
         *
         * 1 = Name
         * 6 = Feature Class
         * 7 = Feature Code
         * 10 = Population
         */
        char *name =
            fields[1];

        char *featureClass =
            fields[6];

        char *featureCode =
            fields[7];

        long population =
            atol(fields[10]);


        /*
         * Keep only useful locations.
         */
        if (!isUsefulLocation(
                featureClass,
                featureCode
            ))
        {
            continue;
        }


        /*
         * Prevent overflow.
         */
        if (
            locationCount >=
            MAX_LOCATIONS
        )
        {
            printf(
                "Maximum location limit reached.\n"
            );

            break;
        }


        /*
         * Store location.
         */
        strncpy(
            locations[
                locationCount
            ].name,
            name,
            NAME_SIZE - 1
        );

        locations[
            locationCount
        ].name[
            NAME_SIZE - 1
        ] = '\0';


        strncpy(
            locations[
                locationCount
            ].featureCode,
            featureCode,
            19
        );

        locations[
            locationCount
        ].featureCode[19] =
            '\0';


        locations[
            locationCount
        ].population =
            population;


        locationCount++;


        /*
         * Progress display.
         */
        if (
            locationCount % 500000 ==
            0
        )
        {
            printf(
                "Loaded %ld locations...\n",
                locationCount
            );
        }
    }


    fclose(file);


    printf("\n");
    printf(
        "Total locations loaded: %ld\n",
        locationCount
    );


    /*
     * Test lookup.
     */
    char searchName[NAME_SIZE];

    printf("\n");
    printf(
        "Enter location to search: "
    );

    fgets(
        searchName,
        sizeof(searchName),
        stdin
    );


    searchName[
        strcspn(
            searchName,
            "\n"
        )
    ] = '\0';


    int found = 0;


    /*
     * Search loaded locations.
     */
    for (
        long i = 0;
        i < locationCount;
        i++
    )
    {
        if (
            caseInsensitiveCompare(
                searchName,
                locations[i].name
            )
        )
        {
            printf("\n");
            printf(
                "Location found!\n"
            );

            printf(
                "Name       : %s\n",
                locations[i].name
            );

            printf(
                "Feature    : %s\n",
                locations[i].featureCode
            );

            printf(
                "Population : %ld\n",
                locations[i].population
            );

            found = 1;

            break;
        }
    }


    if (!found)
    {
        printf(
            "\nLocation not found.\n"
        );
    }


    free(locations);

    return 0;
}