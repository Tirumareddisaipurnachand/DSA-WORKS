#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LINE_SIZE 16384
#define MAX_FIELDS 19
#define NAME_SIZE 256
#define ALT_NAME_SIZE 8192


/*
 * Convert an uppercase English character to lowercase.
 */
char toLowerChar(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c + ('a' - 'A');

    return c;
}


/*
 * Case-insensitive string comparison.
 */
int caseInsensitiveCompare(const char *a, const char *b)
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
 * Split a line using TAB characters.
 *
 * Empty fields are preserved.
 */
int split_tab(char *line, char *fields[])
{
    int count = 0;

    char *start = line;

    for (char *p = line; *p != '\0'; p++)
    {
        if (*p == '\t')
        {
            *p = '\0';

            if (count < MAX_FIELDS)
                fields[count++] = start;

            start = p + 1;
        }

        else if (*p == '\n' || *p == '\r')
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
 * Return the minimum of three numbers.
 */
int minimum(int a, int b, int c)
{
    int min = a;

    if (b < min)
        min = b;

    if (c < min)
        min = c;

    return min;
}


/*
 * Calculate Levenshtein edit distance.
 */
int levenshteinDistance(const char *a, const char *b)
{
    int lenA = strlen(a);
    int lenB = strlen(b);

    int *previous =
        malloc((lenB + 1) * sizeof(int));

    int *current =
        malloc((lenB + 1) * sizeof(int));

    if (previous == NULL || current == NULL)
    {
        free(previous);
        free(current);

        return -1;
    }


    /*
     * Initialize first row.
     */
    for (int j = 0; j <= lenB; j++)
    {
        previous[j] = j;
    }


    /*
     * Calculate edit distance.
     */
    for (int i = 1; i <= lenA; i++)
    {
        current[0] = i;

        for (int j = 1; j <= lenB; j++)
        {
            int cost;

            if (toLowerChar(a[i - 1]) ==
                toLowerChar(b[j - 1]))
            {
                cost = 0;
            }
            else
            {
                cost = 1;
            }

            current[j] = minimum(
                previous[j] + 1,
                current[j - 1] + 1,
                previous[j - 1] + cost
            );
        }


        /*
         * Swap rows.
         */
        int *temp = previous;

        previous = current;

        current = temp;
    }


    int distance = previous[lenB];

    free(previous);
    free(current);

    return distance;
}


/*
 * Determine the maximum fuzzy distance allowed.
 */
int getMaximumDistance(const char *searchName)
{
    int length = strlen(searchName);

    if (length <= 4)
        return 1;

    if (length <= 7)
        return 2;

    return 3;
}


/*
 * Determine location importance.
 *
 * Higher number = more important.
 *
 * PPLC = national capital
 * PPLA = administrative capital
 * PPL  = populated place
 * ADM1 = first-level administrative division
 * ADM2 = second-level administrative division
 * ADM  = other administrative division
 */
int getFeaturePriority(const char *featureCode)
{
    /*
     * National capital
     */
    if (strncmp(featureCode, "PPLC", 4) == 0)
        return 5;


    /*
     * Administrative capital
     */
    if (strncmp(featureCode, "PPLA", 4) == 0)
        return 4;


    /*
     * Populated place
     */
    if (strncmp(featureCode, "PPL", 3) == 0)
        return 3;


    /*
     * First-level administrative division
     */
    if (strncmp(featureCode, "ADM1", 4) == 0)
        return 2;


    /*
     * Second-level administrative division
     *
     * Also covers other ADM records.
     */
    if (strncmp(featureCode, "ADM2", 4) == 0)
        return 1;

    if (strncmp(featureCode, "ADM", 3) == 0)
        return 1;


    return 0;
}


/*
 * Search the alternate names of one record.
 *
 * Returns:
 *
 * 1 = exact alternate-name match
 * 0 = no match
 */
int hasExactAlternateName(
    const char *alternateNames,
    const char *searchName,
    char *matchedName)
{
    if (alternateNames == NULL ||
        alternateNames[0] == '\0')
    {
        return 0;
    }


    /*
     * Copy because strtok modifies the string.
     */
    char copy[ALT_NAME_SIZE];

    strncpy(
        copy,
        alternateNames,
        ALT_NAME_SIZE - 1
    );

    copy[ALT_NAME_SIZE - 1] = '\0';


    /*
     * Split alternate names by comma.
     */
    char *token = strtok(copy, ",");

    while (token != NULL)
    {
        if (caseInsensitiveCompare(
                token,
                searchName))
        {
            strncpy(
                matchedName,
                token,
                NAME_SIZE - 1
            );

            matchedName[NAME_SIZE - 1] = '\0';

            return 1;
        }

        token = strtok(NULL, ",");
    }

    return 0;
}


/*
 * Search one location name.
 */
void searchLocation(const char *searchName)
{
    FILE *file =
        fopen("data/locations.txt", "r");

    if (file == NULL)
    {
        printf(
            "Error: Could not open data/locations.txt\n"
        );

        return;
    }


    char line[LINE_SIZE];

    char *fields[MAX_FIELDS];


    /*
     * Best candidate information.
     */
    char bestMatch[NAME_SIZE] = "";

    char bestCanonicalName[NAME_SIZE] = "";

    char bestFeatureCode[NAME_SIZE] = "";

    int bestDistance = 999999;

    int bestMatchType = 0;

    int bestFeaturePriority = -1;

    long bestPopulation = -1;


    /*
     * Count records checked.
     */
    long checkedLocations = 0;


    /*
     * Maximum allowed fuzzy distance.
     */
    int maximumDistance =
        getMaximumDistance(searchName);


    /*
     * =====================================================
     * FIRST PASS
     *
     * Search for exact canonical names and exact
     * alternate names.
     *
     * IMPORTANT:
     * Canonical and alternate names compete equally.
     *
     * Ranking:
     *
     * 1. Feature priority
     * 2. Population
     *
     * This prevents a small ADM2 "Bombay" from beating
     * Mumbai's alternate name "Bombay".
     * =====================================================
     */

    while (fgets(
        line,
        sizeof(line),
        file
    ) != NULL)
    {
        int fieldCount =
            split_tab(line, fields);

        if (fieldCount < 11)
            continue;


        /*
         * locations.txt format:
         *
         * 0 ID
         * 1 Name
         * 2 ASCII Name
         * 3 Alternate Names
         * 4 Latitude
         * 5 Longitude
         * 6 Feature Class
         * 7 Feature Code
         * 8 Country
         * 9 Admin1
         * 10 Population
         */

        char *name = fields[1];

        char *alternateNames = fields[3];

        char *featureCode = fields[7];

        long population =
            atol(fields[10]);

        int featurePriority =
            getFeaturePriority(featureCode);


        /*
         * --------------------------------------------
         * Exact canonical name
         * --------------------------------------------
         */
        if (caseInsensitiveCompare(
                name,
                searchName))
        {
            int shouldReplace = 0;


            /*
             * Better feature priority.
             */
            if (featurePriority >
                bestFeaturePriority)
            {
                shouldReplace = 1;
            }


            /*
             * Same feature priority:
             * prefer larger population.
             */
            else if (
                featurePriority ==
                    bestFeaturePriority &&
                population >
                    bestPopulation
            )
            {
                shouldReplace = 1;
            }


            if (shouldReplace)
            {
                strcpy(
                    bestMatch,
                    name
                );

                strcpy(
                    bestCanonicalName,
                    name
                );

                strcpy(
                    bestFeatureCode,
                    featureCode
                );

                bestDistance = 0;

                bestMatchType = 3;

                bestFeaturePriority =
                    featurePriority;

                bestPopulation =
                    population;
            }
        }


        /*
         * --------------------------------------------
         * Exact alternate name
         * --------------------------------------------
         */
        char matchedAlternate[NAME_SIZE];

        if (hasExactAlternateName(
                alternateNames,
                searchName,
                matchedAlternate))
        {
            int shouldReplace = 0;


            /*
             * Better feature priority.
             */
            if (featurePriority >
                bestFeaturePriority)
            {
                shouldReplace = 1;
            }


            /*
             * Same feature priority:
             * prefer larger population.
             */
            else if (
                featurePriority ==
                    bestFeaturePriority &&
                population >
                    bestPopulation
            )
            {
                shouldReplace = 1;
            }


            if (shouldReplace)
            {
                strcpy(
                    bestMatch,
                    matchedAlternate
                );

                strcpy(
                    bestCanonicalName,
                    name
                );

                strcpy(
                    bestFeatureCode,
                    featureCode
                );

                bestDistance = 0;

                bestMatchType = 2;

                bestFeaturePriority =
                    featurePriority;

                bestPopulation =
                    population;
            }
        }


        checkedLocations++;


        /*
         * Progress display.
         */
        if (checkedLocations % 1000000 == 0)
        {
            printf(
                "Checked %ld locations...\n",
                checkedLocations
            );
        }
    }


    /*
     * If an exact match was found,
     * display the best result.
     */
    if (bestMatchType >= 2)
    {
        printf("\n");

        printf(
            "Location found!\n"
        );

        printf(
            "------------------------------\n"
        );

        printf(
            "Input      : %s\n",
            searchName
        );


        /*
         * Canonical exact match.
         */
        if (bestMatchType == 3)
        {
            printf(
                "Match      : %s\n",
                bestMatch
            );
        }


        /*
         * Alternate exact match.
         */
        else
        {
            printf(
                "Matched as : %s\n",
                bestMatch
            );

            printf(
                "Canonical  : %s\n",
                bestCanonicalName
            );
        }


        printf(
            "Distance   : %d\n",
            bestDistance
        );

        printf(
            "Population : %ld\n",
            bestPopulation
        );

        printf(
            "Feature    : %s\n",
            bestFeatureCode
        );

        printf(
            "------------------------------\n"
        );


        fclose(file);

        return;
    }


    /*
     * =====================================================
     * SECOND PASS
     *
     * No exact match was found.
     *
     * Perform fuzzy matching.
     * =====================================================
     */

    rewind(file);

    checkedLocations = 0;


    /*
     * Reset best candidate.
     */
    bestMatch[0] = '\0';

    bestCanonicalName[0] = '\0';

    bestFeatureCode[0] = '\0';

    bestDistance = 999999;

    bestMatchType = 0;

    bestFeaturePriority = -1;

    bestPopulation = -1;


    while (fgets(
        line,
        sizeof(line),
        file
    ) != NULL)
    {
        int fieldCount =
            split_tab(line, fields);

        if (fieldCount < 11)
            continue;


        char *name = fields[1];

        char *alternateNames = fields[3];

        char *featureCode = fields[7];

        long population =
            atol(fields[10]);


        int featurePriority =
            getFeaturePriority(featureCode);


        /*
         * --------------------------------------------
         * Fuzzy match canonical name
         * --------------------------------------------
         */
        int distance =
            levenshteinDistance(
                searchName,
                name
            );


        if (
            distance >= 0 &&
            distance <= maximumDistance
        )
        {
            int shouldReplace = 0;


            /*
             * Better distance.
             */
            if (distance < bestDistance)
            {
                shouldReplace = 1;
            }


            /*
             * Same distance:
             * prefer better feature.
             */
            else if (
                distance == bestDistance &&
                featurePriority >
                    bestFeaturePriority
            )
            {
                shouldReplace = 1;
            }


            /*
             * Same distance and feature:
             * prefer higher population.
             */
            else if (
                distance == bestDistance &&
                featurePriority ==
                    bestFeaturePriority &&
                population >
                    bestPopulation
            )
            {
                shouldReplace = 1;
            }


            if (shouldReplace)
            {
                bestDistance =
                    distance;

                bestPopulation =
                    population;

                bestFeaturePriority =
                    featurePriority;

                bestMatchType = 1;

                strcpy(
                    bestMatch,
                    name
                );

                strcpy(
                    bestCanonicalName,
                    name
                );

                strcpy(
                    bestFeatureCode,
                    featureCode
                );
            }
        }


        /*
         * --------------------------------------------
         * Fuzzy match alternate names
         * --------------------------------------------
         */
        if (alternateNames[0] != '\0')
        {
            char altCopy[ALT_NAME_SIZE];

            strncpy(
                altCopy,
                alternateNames,
                ALT_NAME_SIZE - 1
            );

            altCopy[ALT_NAME_SIZE - 1] =
                '\0';


            char *token =
                strtok(altCopy, ",");


            while (token != NULL)
            {
                int altDistance =
                    levenshteinDistance(
                        searchName,
                        token
                    );


                if (
                    altDistance >= 0 &&
                    altDistance <= maximumDistance
                )
                {
                    int shouldReplace = 0;


                    /*
                     * Better distance.
                     */
                    if (
                        altDistance <
                        bestDistance
                    )
                    {
                        shouldReplace = 1;
                    }


                    /*
                     * Same distance:
                     * prefer better feature.
                     */
                    else if (
                        altDistance ==
                            bestDistance &&
                        featurePriority >
                            bestFeaturePriority
                    )
                    {
                        shouldReplace = 1;
                    }


                    /*
                     * Same distance and feature:
                     * prefer higher population.
                     */
                    else if (
                        altDistance ==
                            bestDistance &&
                        featurePriority ==
                            bestFeaturePriority &&
                        population >
                            bestPopulation
                    )
                    {
                        shouldReplace = 1;
                    }


                    if (shouldReplace)
                    {
                        bestDistance =
                            altDistance;

                        bestPopulation =
                            population;

                        bestFeaturePriority =
                            featurePriority;

                        bestMatchType = 1;


                        strcpy(
                            bestMatch,
                            token
                        );


                        strcpy(
                            bestCanonicalName,
                            name
                        );


                        strcpy(
                            bestFeatureCode,
                            featureCode
                        );
                    }
                }


                token = strtok(
                    NULL,
                    ","
                );
            }
        }


        checkedLocations++;


        if (checkedLocations % 1000000 == 0)
        {
            printf(
                "Checked %ld locations...\n",
                checkedLocations
            );
        }
    }


    fclose(file);


    /*
     * =====================================================
     * DISPLAY FUZZY RESULT
     * =====================================================
     */

    if (bestMatch[0] != '\0')
    {
        printf("\n");

        printf(
            "Closest location found!\n"
        );

        printf(
            "------------------------------\n"
        );

        printf(
            "Input      : %s\n",
            searchName
        );


        /*
         * Fuzzy match is an alternate name.
         */
        if (
            !caseInsensitiveCompare(
                bestMatch,
                bestCanonicalName
            )
        )
        {
            printf(
                "Suggested  : %s\n",
                bestMatch
            );

            printf(
                "Canonical  : %s\n",
                bestCanonicalName
            );
        }
        else
        {
            printf(
                "Suggested  : %s\n",
                bestCanonicalName
            );
        }


        printf(
            "Distance   : %d\n",
            bestDistance
        );

        printf(
            "Population : %ld\n",
            bestPopulation
        );

        printf(
            "Feature    : %s\n",
            bestFeatureCode
        );

        printf(
            "------------------------------\n"
        );
    }
    else
    {
        printf("\n");

        printf(
            "No close location found.\n"
        );
    }
}


/*
 * Main function.
 */
int main()
{
    char searchName[NAME_SIZE];


    printf(
        "========================================\n"
    );

    printf(
        "       LOCATION SEARCH SYSTEM\n"
    );

    printf(
        "========================================\n\n"
    );


    printf(
        "Enter location name: "
    );


    fgets(
        searchName,
        sizeof(searchName),
        stdin
    );


    /*
     * Remove newline.
     */
    searchName[
        strcspn(searchName, "\n")
    ] = '\0';


    /*
     * Prevent empty search.
     */
    if (searchName[0] == '\0')
    {
        printf(
            "\nPlease enter a location name.\n"
        );

        return 0;
    }


    searchLocation(searchName);


    return 0;
}