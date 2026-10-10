#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LINE_SIZE 16384
#define MAX_FIELDS 19
#define NAME_SIZE 256
#define SENTENCE_SIZE 4096
#define MAX_WORDS 700

typedef struct {
    char name[NAME_SIZE];
    char featureCode[20];
    long population;
} Location;

typedef struct {
    int start;
    int end;
    char text[NAME_SIZE];
} Word;

static Location *locations = NULL;
static size_t locationCount = 0;
static size_t locationCapacity = 0;

static char lowerChar(char c) {
    return (char)tolower((unsigned char)c);
}

static int equalsIgnoreCase(const char *a, const char *b) {
    while (*a && *b) {
        if (lowerChar(*a) != lowerChar(*b))
            return 0;
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

static int compareNames(const void *left, const void *right) {
    const Location *a = (const Location *)left;
    const Location *b = (const Location *)right;

    const unsigned char *pa = (const unsigned char *)a->name;
    const unsigned char *pb = (const unsigned char *)b->name;

    while (*pa && *pb) {
        int ca = tolower(*pa);
        int cb = tolower(*pb);

        if (ca != cb)
            return ca - cb;

        pa++;
        pb++;
    }

    return (int)*pa - (int)*pb;
}

static int compareNameToText(const char *a, const char *b) {
    while (*a && *b) {
        int ca = lowerChar(*a);
        int cb = lowerChar(*b);

        if (ca != cb)
            return ca - cb;

        a++;
        b++;
    }

    if (*a)
        return 1;

    if (*b)
        return -1;

    return 0;
}

static int splitTabs(char *line, char **fields, int maxFields) {
    int count = 0;
    char *start = line;

    for (char *p = line; *p; ++p) {
        if (*p == '\t') {
            *p = '\0';

            if (count < maxFields)
                fields[count++] = start;

            start = p + 1;
        } else if (*p == '\n' || *p == '\r') {
            *p = '\0';
            break;
        }
    }

    if (count < maxFields)
        fields[count++] = start;

    return count;
}

static int isUsefulLocation(const char *featureClass,
                            const char *featureCode) {
    if (featureClass[0] == 'A')
        return 1;

    if (featureClass[0] == 'P' &&
        (strncmp(featureCode, "PPL", 3) == 0 ||
         strncmp(featureCode, "PCLI", 4) == 0))
        return 1;

    return 0;
}

static int featurePriority(const char *code) {
    if (strncmp(code, "PPLC", 4) == 0)
        return 5;

    if (strncmp(code, "PPLA", 4) == 0)
        return 4;

    if (strncmp(code, "PPL", 3) == 0)
        return 3;

    if (strncmp(code, "ADM1", 4) == 0)
        return 2;

    if (strncmp(code, "ADM", 3) == 0)
        return 1;

    return 0;
}

static int addLocation(const char *name,
                       const char *code,
                       long population) {
    if (!name || !*name || strlen(name) >= NAME_SIZE)
        return 1;

    if (locationCount == locationCapacity) {
        size_t newCapacity =
            locationCapacity ? locationCapacity * 2 : 50000;

        Location *tmp = realloc(
            locations, newCapacity * sizeof(*locations)
        );

        if (!tmp)
            return 0;

        locations = tmp;
        locationCapacity = newCapacity;
    }

    snprintf(locations[locationCount].name,
             NAME_SIZE, "%s", name);

    snprintf(locations[locationCount].featureCode,
             sizeof(locations[locationCount].featureCode),
             "%s", code);

    locations[locationCount].population = population;
    locationCount++;

    return 1;
}

static long loadLocations(void) {
    FILE *file = fopen("data/locations.txt", "r");

    if (!file) {
        fprintf(stderr,
                "Error: Cannot open data/locations.txt. "
                "Run the program from your project folder.\n");
        return -1;
    }

    char line[LINE_SIZE];
    long rows = 0;

    while (fgets(line, sizeof(line), file)) {
        char *fields[MAX_FIELDS];

        int n = splitTabs(line, fields, MAX_FIELDS);

        if (n < 11 ||
            !isUsefulLocation(fields[6], fields[7]))
            continue;

        if (!addLocation(fields[1],
                         fields[7],
                         atol(fields[10]))) {
            fprintf(stderr,
                    "Error: Not enough memory while loading locations.\n");
            fclose(file);
            return -1;
        }

        rows++;

        if (rows % 500000 == 0)
            printf("Loaded %ld locations...\n", rows);
    }

    fclose(file);

    qsort(locations, locationCount,
          sizeof(*locations), compareNames);

    return (long)locationCount;
}

static int isCommonWord(const char *word) {
    static const char *words[] = {
        "i", "im", "iam", "me", "my", "you", "your",
        "he", "she", "we", "they", "it",
        "a", "an", "the", "is", "am", "are", "was",
        "were", "be", "been", "being",
        "do", "does", "did", "have", "has", "had",
        "in", "into", "on", "onto", "at", "to", "from",
        "by", "for", "of", "with", "about", "near",
        "and", "or", "but", "if", "then",
        "this", "that", "these", "those",
        "going", "go", "went", "come", "coming", "came",
        "live", "lives", "living", "lived",
        "work", "works", "working", "worked",
        "stay", "staying", "travel", "traveling",
        "travelling", "visit", "visiting", "visited",
        "away", "km", "kms", "meter", "meters",
        "mile", "miles", "today", "tomorrow", "yesterday",
        "please", "hello", "hi", "name", "called",
        "person", "friend", "school", "college", "office",
        "home", "want", "need", "like", "where", "when",
        "what", "which", "who", "how", "why",
        "not", "no", "yes", NULL
    };

    for (int i = 0; words[i]; ++i) {
        if (equalsIgnoreCase(word, words[i]))
            return 1;
    }

    return 0;
}

static int isContextCue(const char *word) {
    static const char *cues[] = {
        "in", "to", "from", "at", "near", "toward",
        "towards", "within", "outside", "inside",
        "around", "through", "across", "visit",
        "visiting", "reach", "reached", "travel",
        "traveling", "travelling", "live", "living",
        "stay", "work", "workin", NULL
    };

    for (int i = 0; cues[i]; ++i) {
        if (equalsIgnoreCase(word, cues[i]))
            return 1;
    }

    return 0;
}

static int levenshtein(const char *a, const char *b) {
    size_t la = strlen(a);
    size_t lb = strlen(b);

    if (la > 100 || lb > 100)
        return 999;

    int prev[101], curr[101];

    for (size_t j = 0; j <= lb; ++j)
        prev[j] = (int)j;

    for (size_t i = 1; i <= la; ++i) {
        curr[0] = (int)i;

        for (size_t j = 1; j <= lb; ++j) {
            int cost =
                lowerChar(a[i - 1]) == lowerChar(b[j - 1])
                ? 0 : 1;

            int ins = curr[j - 1] + 1;
            int del = prev[j] + 1;
            int sub = prev[j - 1] + cost;

            int best = ins < del ? ins : del;
            curr[j] = sub < best ? sub : best;
        }

        for (size_t j = 0; j <= lb; ++j)
            prev[j] = curr[j];
    }

    return prev[lb];
}

static int findExactCanonical(const char *text, char *result) {
    size_t low = 0;
    size_t high = locationCount;

    while (low < high) {
        size_t mid = low + (high - low) / 2;

        int cmp = compareNameToText(
            locations[mid].name, text
        );

        if (cmp < 0)
            low = mid + 1;
        else
            high = mid;
    }

    int found = 0;
    int bestP = -1;
    long bestPop = -1;

    for (size_t i = low;
         i < locationCount &&
         compareNameToText(locations[i].name, text) == 0;
         ++i) {

        int p = featurePriority(locations[i].featureCode);

        if (!found ||
            p > bestP ||
            (p == bestP &&
             locations[i].population > bestPop)) {

            snprintf(result, NAME_SIZE, "%s",
                     locations[i].name);

            bestP = p;
            bestPop = locations[i].population;
            found = 1;
        }
    }

    return found;
}

/* Historical names and common misspellings mapped to preferred names. */
static int findPreferredModernName(const char *phrase,
                                   char *canonical) {
    static const struct {
        const char *oldName;
        const char *modernName;
    } names[] = {
        {"bombay", "Mumbai"},
        {"bombai", "Mumbai"},
        {"bomaby", "Mumbai"},
        {"vizag", "Visakhapatnam"},
        {"koria", "Korea"},
        {"korea", "Korea"},
        {NULL, NULL}
    };

    for (int i = 0; names[i].oldName; ++i) {
        if (equalsIgnoreCase(phrase, names[i].oldName)) {
            snprintf(canonical, NAME_SIZE, "%s",
                     names[i].modernName);
            return 1;
        }
    }

    return 0;
}

/* Searches canonical names, ASCII names and alternate names in the TSV. */
static int findExactAliasOrAscii(const char *phrase,
                                 char *canonical) {
    FILE *file = fopen("data/locations.txt", "r");

    if (!file)
        return 0;

    char line[LINE_SIZE];

    int found = 0;
    int bestP = -1;
    long bestPop = -1;

    while (fgets(line, sizeof(line), file)) {
        char *fields[MAX_FIELDS];

        int n = splitTabs(line, fields, MAX_FIELDS);

        if (n < 11 ||
            !isUsefulLocation(fields[6], fields[7]))
            continue;

        int match =
            equalsIgnoreCase(phrase, fields[1]) ||
            equalsIgnoreCase(phrase, fields[2]);

        if (!match && fields[3][0]) {
            char altCopy[LINE_SIZE];

            snprintf(altCopy, sizeof(altCopy), "%s",
                     fields[3]);

            for (char *alt = strtok(altCopy, ",");
                 alt;
                 alt = strtok(NULL, ",")) {

                while (*alt &&
                       isspace((unsigned char)*alt))
                    alt++;

                char *end = alt + strlen(alt);

                while (end > alt &&
                       isspace((unsigned char)end[-1]))
                    *--end = '\0';

                if (equalsIgnoreCase(phrase, alt)) {
                    match = 1;
                    break;
                }
            }
        }

        if (match) {
            int p = featurePriority(fields[7]);
            long pop = atol(fields[10]);

            if (!found ||
                p > bestP ||
                (p == bestP && pop > bestPop)) {

                snprintf(canonical, NAME_SIZE, "%s",
                         fields[1]);

                bestP = p;
                bestPop = pop;
                found = 1;
            }
        }
    }

    fclose(file);
    return found;
}

static int findFuzzy(const char *word,
                     char *result,
                     int *distance) {
    int len = (int)strlen(word);

    if (len < 5)
        return 0;

    int allowed = len <= 6 ? 1 : 2;
    int bestD = allowed + 1;
    int bestP = -1;
    int found = 0;
    long bestPop = -1;

    for (size_t i = 0; i < locationCount; ++i) {
        int nameLen = (int)strlen(locations[i].name);

        if (nameLen < 4 ||
            nameLen > 100 ||
            abs(nameLen - len) > allowed)
            continue;

        int d = levenshtein(word, locations[i].name);

        if (d > allowed)
            continue;

        int p = featurePriority(locations[i].featureCode);

        if (!found ||
            d < bestD ||
            (d == bestD && p > bestP) ||
            (d == bestD && p == bestP &&
             locations[i].population > bestPop)) {

            snprintf(result, NAME_SIZE, "%s",
                     locations[i].name);

            bestD = d;
            bestP = p;
            bestPop = locations[i].population;
            found = 1;
        }
    }

    if (found)
        *distance = bestD;

    return found;
}

static int tokenize(const char *sentence,
                    Word words[],
                    int maxWords) {
    int count = 0;
    int i = 0;

    while (sentence[i] && count < maxWords) {
        if (isalpha((unsigned char)sentence[i])) {
            int start = i;
            int w = 0;
            char token[NAME_SIZE];

            while (isalpha((unsigned char)sentence[i])) {
                if (w < NAME_SIZE - 1)
                    token[w++] = sentence[i];

                i++;
            }

            token[w] = '\0';

            words[count].start = start;
            words[count].end = i;

            snprintf(words[count].text, NAME_SIZE,
                     "%s", token);

            count++;
        } else {
            i++;
        }
    }

    return count;
}

static void processSentence(const char *sentence) {
    Word words[MAX_WORDS];

    int count = tokenize(sentence, words, MAX_WORDS);

    int replaceStart[MAX_WORDS];
    int replaceEnd[MAX_WORDS];

    char replacement[MAX_WORDS][NAME_SIZE];

    for (int i = 0; i < count; ++i) {
        replaceStart[i] = -1;
        replaceEnd[i] = -1;
        replacement[i][0] = '\0';
    }

    printf("\nDetected locations:\n");
    printf("------------------------------\n");

    for (int i = 0; i < count; ++i) {
        if (isCommonWord(words[i].text))
            continue;

        int context =
            i > 0 && isContextCue(words[i - 1].text);

        int matchedSpan = 0;

        int maxSpan =
            (count - i < 5) ? count - i : 5;

        /* Prefer the longest exact canonical place name. */
        for (int span = maxSpan; span >= 1; --span) {
            int start = words[i].start;
            int end = words[i + span - 1].end;
            int len = end - start;

            char phrase[NAME_SIZE];
            char canonical[NAME_SIZE];

            if (len <= 0 || len >= NAME_SIZE)
                continue;

            memcpy(phrase, sentence + start, (size_t)len);
            phrase[len] = '\0';

            if (context &&
                findPreferredModernName(phrase, canonical)) {

                if (strcmp(phrase, canonical) != 0) {
                    replaceStart[i] = i;
                    replaceEnd[i] = i + span - 1;

                    snprintf(replacement[i], NAME_SIZE,
                             "%s", canonical);

                    printf("%s -> %s (preferred modern name)\n",
                           phrase, canonical);
                }

                matchedSpan = span;
                break;
            }

            if (findExactCanonical(phrase, canonical)) {
                if (strcmp(phrase, canonical) != 0) {
                    replaceStart[i] = i;
                    replaceEnd[i] = i + span - 1;

                    snprintf(replacement[i], NAME_SIZE,
                             "%s", canonical);

                    printf("%s -> %s\n", phrase, canonical);
                }

                matchedSpan = span;
                break;
            }
        }

        /* Check aliases and ASCII names in location context. */
        if (!matchedSpan && context) {
            for (int span = 1; span <= maxSpan; ++span) {
                int start = words[i].start;
                int end = words[i + span - 1].end;
                int len = end - start;

                char phrase[NAME_SIZE];
                char canonical[NAME_SIZE];

                if (len <= 0 || len >= NAME_SIZE)
                    continue;

                memcpy(phrase, sentence + start, (size_t)len);
                phrase[len] = '\0';

                if (findExactAliasOrAscii(phrase, canonical)) {
                    if (strcmp(phrase, canonical) != 0) {
                        replaceStart[i] = i;
                        replaceEnd[i] = i + span - 1;

                        snprintf(replacement[i], NAME_SIZE,
                                 "%s", canonical);

                        printf("%s -> %s\n", phrase, canonical);
                    }

                    matchedSpan = span;
                    break;
                }
            }
        }

        if (matchedSpan > 0) {
            i += matchedSpan - 1;
            continue;
        }

        /* Only attempt fuzzy correction after a location cue. */
        if (context && strlen(words[i].text) >= 5) {
            char corrected[NAME_SIZE];
            int distance = 0;

            if (findFuzzy(words[i].text,
                          corrected, &distance)) {

                replaceStart[i] = i;
                replaceEnd[i] = i;

                snprintf(replacement[i], NAME_SIZE,
                         "%s", corrected);

                printf("%s -> %s (spelling distance %d)\n",
                       words[i].text, corrected, distance);
            }
        }
    }

    char output[SENTENCE_SIZE];

    int out = 0;
    int pos = 0;
    int wi = 0;

    while (sentence[pos] && out < SENTENCE_SIZE - 1) {
        if (wi < count &&
            pos == words[wi].start &&
            replaceStart[wi] == wi) {

            int endWord = replaceEnd[wi];
            int len = (int)strlen(replacement[wi]);

            if (out + len >= SENTENCE_SIZE)
                len = SENTENCE_SIZE - out - 1;

            memcpy(output + out, replacement[wi], (size_t)len);
            out += len;

            pos = words[endWord].end;
            wi = endWord + 1;
        } else {
            output[out++] = sentence[pos++];

            if (wi < count && pos >= words[wi].end)
                wi++;
        }
    }

    output[out] = '\0';

    printf("------------------------------\n");
    printf("Corrected sentence:\n%s\n", output);
}

int main(void) {
    printf("========================================\n");
    printf("GEOSPATIAL SENTENCE PROCESSOR\n");
    printf("========================================\n\n");
    printf("Loading location database...\n");

    long count = loadLocations();

    if (count <= 0) {
        free(locations);
        return 1;
    }

    printf("\nTotal locations loaded: %ld\n", count);
    printf("\nEnter a sentence:\n");

    char sentence[SENTENCE_SIZE];

    if (!fgets(sentence, sizeof(sentence), stdin)) {
        free(locations);
        return 1;
    }

    sentence[strcspn(sentence, "\r\n")] = '\0';

    if (!sentence[0]) {
        puts("Please enter a sentence.");
        free(locations);
        return 0;
    }

    printf("\nOriginal sentence:\n%s\n", sentence);

    processSentence(sentence);

    free(locations);
    return 0;
}