#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "infra.h"


/* =========================================================
   INTERNAL HELPER FUNCTIONS
   ========================================================= */

static int increase_capacity(Document *document)
{
    char **temporary;
    int new_capacity;

    new_capacity = document->capacity * 2;

    temporary = realloc(
        document->lines,
        sizeof(char *) * new_capacity
    );

    if (temporary == NULL) {
        return -1;
    }

    document->lines = temporary;
    document->capacity = new_capacity;

    return 0;
}


static void remove_newline(char *text)
{
    text[strcspn(text, "\n")] = '\0';
}


static void free_lines(char **lines, int line_count)
{
    int i;

    if (lines == NULL) {
        return;
    }

    for (i = 0; i < line_count; i++) {
        free(lines[i]);
    }

    free(lines);
}


static void clear_undo_state_internal(Document *document)
{
    free_lines(
        document->undo_lines,
        document->undo_line_count
    );

    document->undo_lines = NULL;
    document->undo_line_count = 0;
    document->undo_capacity = 0;
    document->has_undo = 0;
}


/* =========================================================
   INITIALIZATION
   ========================================================= */

void initialize_editor(Document *document)
{
    document->lines = malloc(
        sizeof(char *) * INITIAL_CAPACITY
    );

    if (document->lines == NULL) {
        printf("Error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    document->line_count = 0;
    document->capacity = INITIAL_CAPACITY;

    document->filename[0] = '\0';

    document->undo_lines = NULL;
    document->undo_line_count = 0;
    document->undo_capacity = 0;
    document->has_undo = 0;
}


/* =========================================================
   UNDO
   ========================================================= */

void clear_undo_state(Document *document)
{
    clear_undo_state_internal(document);
}


int save_undo_state(Document *document)
{
    char **snapshot;
    int snapshot_capacity;
    int i;

    clear_undo_state_internal(document);

    snapshot_capacity = document->capacity;

    if (snapshot_capacity < INITIAL_CAPACITY) {
        snapshot_capacity = INITIAL_CAPACITY;
    }

    snapshot = malloc(
        sizeof(char *) * snapshot_capacity
    );

    if (snapshot == NULL) {
        return -1;
    }

    for (i = 0; i < document->line_count; i++) {
        size_t length;

        length = strlen(document->lines[i]);

        snapshot[i] = malloc(length + 1);

        if (snapshot[i] == NULL) {
            free_lines(snapshot, i);
            return -1;
        }

        strcpy(
            snapshot[i],
            document->lines[i]
        );
    }

    for (i = document->line_count;
         i < snapshot_capacity;
         i++) {

        snapshot[i] = NULL;
    }

    document->undo_lines = snapshot;
    document->undo_line_count = document->line_count;
    document->undo_capacity = snapshot_capacity;
    document->has_undo = 1;

    return 0;
}


int undo_last_action(Document *document)
{
    char **current_lines;
    int current_line_count;

    if (!document->has_undo) {
        return -1;
    }

    current_lines = document->lines;
    current_line_count = document->line_count;

    document->lines = document->undo_lines;
    document->line_count = document->undo_line_count;
    document->capacity = document->undo_capacity;

    document->undo_lines = NULL;
    document->undo_line_count = 0;
    document->undo_capacity = 0;
    document->has_undo = 0;

    free_lines(
        current_lines,
        current_line_count
    );

    return 0;
}


/* =========================================================
   INSERT LINE
   ========================================================= */

int insert_line(
    Document *document,
    const char *text
)
{
    char *new_line;
    size_t length;

    if (text == NULL || text[0] == '\0') {
        return -1;
    }

    if (document->line_count >= document->capacity) {

        if (increase_capacity(document) != 0) {
            return -1;
        }
    }

    length = strlen(text);

    new_line = malloc(length + 1);

    if (new_line == NULL) {
        return -1;
    }

    strcpy(new_line, text);

    document->lines[
        document->line_count
    ] = new_line;

    document->line_count++;

    return 0;
}


/* =========================================================
   DELETE LINE
   ========================================================= */

int delete_line(
    Document *document,
    int line_number
)
{
    int i;

    if (line_number < 0 ||
        line_number >= document->line_count) {

        return -1;
    }

    free(
        document->lines[line_number]
    );

    for (i = line_number;
         i < document->line_count - 1;
         i++) {

        document->lines[i] =
            document->lines[i + 1];
    }

    document->line_count--;

    document->lines[
        document->line_count
    ] = NULL;

    return 0;
}


/* =========================================================
   DISPLAY DOCUMENT
   ========================================================= */

void display_document(
    const Document *document
)
{
    int i;

    if (document->line_count == 0) {

        printf("\nDocument is empty.\n\n");

        return;
    }

    printf("\n");
    printf("====================================\n");
    printf("           DOCUMENT\n");
    printf("====================================\n\n");

    for (i = 0;
         i < document->line_count;
         i++) {

        printf(
            "%d | %s\n",
            i,
            document->lines[i]
        );
    }

    printf("\n");
}


/* =========================================================
   SEARCH
   ========================================================= */

int search_document(
    const Document *document,
    const char *query
)
{
    int i;
    int found = 0;

    if (query == NULL ||
        query[0] == '\0') {

        return -1;
    }

    for (i = 0;
         i < document->line_count;
         i++) {

        if (strstr(
                document->lines[i],
                query
            ) != NULL) {

            printf(
                "Found \"%s\" on line %d.\n",
                query,
                i
            );

            found = 1;
        }
    }

    return found;
}


/* =========================================================
   REPLACE TEXT INSIDE ONE LINE
   ========================================================= */

static int replace_in_line(
    char **line,
    const char *find,
    const char *replace
)
{
    char *position;
    char *result;
    char *source;
    char *destination;

    size_t find_length;
    size_t replace_length;
    size_t original_length;
    size_t new_length;
    size_t part_length;

    int count = 0;


    if (line == NULL ||
        *line == NULL ||
        find == NULL ||
        replace == NULL) {

        return -1;
    }


    find_length = strlen(find);
    replace_length = strlen(replace);
    original_length = strlen(*line);


    if (find_length == 0) {
        return 0;
    }


    /*
     * Count occurrences.
     */
    position = *line;

    while ((position = strstr(
                position,
                find
            )) != NULL) {

        count++;

        position += find_length;
    }


    if (count == 0) {
        return 0;
    }


    /*
     * Check maximum line length.
     */
    if (replace_length >= find_length) {

        size_t increase =
            replace_length - find_length;

        if (increase != 0 &&
            (size_t)count >
                (SIZE_MAX - original_length) /
                increase) {

            return -1;
        }

        new_length =
            original_length +
            increase * (size_t)count;

    } else {

        size_t decrease =
            find_length - replace_length;

        new_length =
            original_length -
            decrease * (size_t)count;
    }


    if (new_length >= MAX_LINE_LENGTH) {
        return -1;
    }


    result = malloc(
        new_length + 1
    );

    if (result == NULL) {
        return -1;
    }


    /*
     * Build the new string.
     */
    source = *line;
    destination = result;


    while ((position = strstr(
                source,
                find
            )) != NULL) {

        part_length =
            (size_t)(position - source);


        memcpy(
            destination,
            source,
            part_length
        );

        destination += part_length;


        memcpy(
            destination,
            replace,
            replace_length
        );

        destination += replace_length;


        source =
            position + find_length;
    }


    /*
     * Copy remaining text.
     */
    strcpy(
        destination,
        source
    );


    /*
     * Replace old string.
     */
    free(*line);

    *line = result;


    return count;
}


/* =========================================================
   FIND AND REPLACE
   ========================================================= */

int find_and_replace(
    Document *document
)
{
    char find[MAX_LINE_LENGTH];
    char replace[MAX_LINE_LENGTH];
    char input[MAX_LINE_LENGTH];

    int matching_count = 0;
    int i;
    int choice;
    int line_number;
    int total_replacements = 0;


    printf("\n====================================\n");
    printf("          FIND AND REPLACE\n");
    printf("====================================\n\n");


    /*
     * Ask what the user wants to find.
     */
    printf(
        "Enter word or phrase to find:\n"
    );

    printf("> ");


    if (fgets(
            find,
            sizeof(find),
            stdin
        ) == NULL) {

        return -1;
    }


    remove_newline(find);


    if (find[0] == '\0') {

        printf(
            "Search text cannot be empty.\n"
        );

        return -1;
    }


    /*
     * Search the entire document.
     */
    printf(
        "\nSearching for \"%s\"...\n\n",
        find
    );


    for (i = 0;
         i < document->line_count;
         i++) {

        if (strstr(
                document->lines[i],
                find
            ) != NULL) {

            matching_count++;

            printf(
                "Line %d: %s\n",
                i,
                document->lines[i]
            );
        }
    }


    /*
     * No matches.
     */
    if (matching_count == 0) {

        printf(
            "No matches found for \"%s\".\n\n",
            find
        );

        return 0;
    }


    /*
     * Give user options.
     */
    printf(
        "\nWhat would you like to do?\n\n"
    );

    printf(
        "1. Replace on one line\n"
    );

    printf(
        "2. Replace all\n"
    );

    printf(
        "3. Cancel\n\n"
    );

    printf("> ");


    if (fgets(
            input,
            sizeof(input),
            stdin
        ) == NULL) {

        return -1;
    }


    if (sscanf(
            input,
            "%d",
            &choice
        ) != 1) {

        printf(
            "Invalid choice.\n"
        );

        return -1;
    }


    if (choice == 3) {

        printf(
            "Find and replace cancelled.\n\n"
        );

        return 0;
    }


    if (choice != 1 &&
        choice != 2) {

        printf(
            "Invalid choice.\n\n"
        );

        return -1;
    }


    /*
     * Replace on one specific line.
     */
    if (choice == 1) {

        printf(
            "\nEnter line number to replace:\n"
        );

        printf("> ");


        if (fgets(
                input,
                sizeof(input),
                stdin
            ) == NULL) {

            return -1;
        }


        if (sscanf(
                input,
                "%d",
                &line_number
            ) != 1) {

            printf(
                "Invalid line number.\n"
            );

            return -1;
        }


        if (line_number < 0 ||
            line_number >= document->line_count) {

            printf(
                "Invalid line number.\n"
            );

            return -1;
        }


        /*
         * Make sure search text exists.
         */
        if (strstr(
                document->lines[line_number],
                find
            ) == NULL) {

            printf(
                "The word \"%s\" was not found "
                "on line %d.\n",
                find,
                line_number
            );

            return -1;
        }
    }


    /*
     * Ask for replacement text.
     */
    printf(
        "\nEnter replacement text:\n"
    );

    printf("> ");


    if (fgets(
            replace,
            sizeof(replace),
            stdin
        ) == NULL) {

        return -1;
    }


    remove_newline(replace);


    /*
     * Replace on one line.
     */
    if (choice == 1) {

        int result;


        result = replace_in_line(
            &document->lines[line_number],
            find,
            replace
        );


        if (result > 0) {

            printf(
                "\nLine %d updated successfully.\n\n",
                line_number
            );

            return 1;

        } else {

            printf(
                "\nError replacing text.\n\n"
            );

            return -1;
        }
    }


    /*
     * Replace across the whole document.
     */
    for (i = 0;
         i < document->line_count;
         i++) {

        int result;


        if (strstr(
                document->lines[i],
                find
            ) != NULL) {

            result = replace_in_line(
                &document->lines[i],
                find,
                replace
            );


            if (result > 0) {

                total_replacements +=
                    result;
            }
        }
    }


    printf(
        "\n%d occurrence(s) replaced successfully.\n\n",
        total_replacements
    );


    if (total_replacements > 0) {
        return 1;
    }

    return 0;
}


/* =========================================================
   DOCUMENT STATISTICS
   ========================================================= */

void show_statistics(
    const Document *document
)
{
    int i;
    int word_count = 0;


    for (i = 0;
         i < document->line_count;
         i++) {

        const char *text =
            document->lines[i];

        int in_word = 0;


        while (*text != '\0') {

            /*
             * Spaces and tabs separate words.
             */
            if (*text == ' ' ||
                *text == '\t') {

                in_word = 0;
            }

            /*
             * Start of a new word.
             */
            else if (in_word == 0) {

                word_count++;
                in_word = 1;
            }

            text++;
        }
    }


    printf("\n====================================\n");
    printf("         DOCUMENT STATISTICS\n");
    printf("====================================\n\n");

    printf(
        "Lines : %d\n",
        document->line_count
    );

    printf(
        "Words : %d\n\n",
        word_count
    );
}


/* =========================================================
   CREATE NEW FILE
   ========================================================= */

int create_new_file(
    Document *document,
    const char *filename
)
{
    FILE *file;


    if (document == NULL ||
        filename == NULL ||
        filename[0] == '\0') {

        return -1;
    }


    /*
     * Check whether file already exists.
     */
    file = fopen(
        filename,
        "r"
    );


    if (file != NULL) {

        fclose(file);

        return 1;
    }


    /*
     * Create a new empty file.
     */
    file = fopen(
        filename,
        "w"
    );


    if (file == NULL) {
        return -1;
    }


    if (fclose(file) != 0) {
        return -1;
    }


    strncpy(
        document->filename,
        filename,
        MAX_FILENAME_LENGTH - 1
    );


    document->filename[
        MAX_FILENAME_LENGTH - 1
    ] = '\0';


    /*
     * New file starts empty.
     */
    clear_undo_state(document);


    return 0;
}


/* =========================================================
   LOAD FILE
   ========================================================= */

int load_file(
    Document *document,
    const char *filename
)
{
    FILE *file;
    char buffer[MAX_LINE_LENGTH];

    char **new_lines;
    int new_capacity;
    int new_count = 0;
    int i;


    file = fopen(
        filename,
        "r"
    );


    if (file == NULL) {
        return -1;
    }


    new_capacity = INITIAL_CAPACITY;


    new_lines = malloc(
        sizeof(char *) * new_capacity
    );


    if (new_lines == NULL) {

        fclose(file);

        return -1;
    }


    while (fgets(
               buffer,
               sizeof(buffer),
               file
           ) != NULL) {

        char *new_line;
        size_t length;


        remove_newline(buffer);


        if (new_count >= new_capacity) {

            char **temporary;

            new_capacity *= 2;

            temporary = realloc(
                new_lines,
                sizeof(char *) * new_capacity
            );


            if (temporary == NULL) {

                fclose(file);

                free_lines(
                    new_lines,
                    new_count
                );

                return -1;
            }


            new_lines = temporary;
        }


        length = strlen(buffer);


        new_line = malloc(
            length + 1
        );


        if (new_line == NULL) {

            fclose(file);

            free_lines(
                new_lines,
                new_count
            );

            return -1;
        }


        strcpy(
            new_line,
            buffer
        );


        new_lines[new_count] =
            new_line;

        new_count++;
    }


    fclose(file);


    /*
     * Replace old document.
     */
    free_lines(
        document->lines,
        document->line_count
    );


    document->lines = new_lines;
    document->line_count = new_count;
    document->capacity = new_capacity;


    strncpy(
        document->filename,
        filename,
        MAX_FILENAME_LENGTH - 1
    );


    document->filename[
        MAX_FILENAME_LENGTH - 1
    ] = '\0';


    /*
     * Loading a file starts a new undo history.
     */
    clear_undo_state(document);


    /*
     * Clear unused pointers.
     */
    for (i = new_count;
         i < new_capacity;
         i++) {

        document->lines[i] = NULL;
    }


    return 0;
}


/* =========================================================
   SAVE FILE
   ========================================================= */

int save_file(
    const Document *document
)
{
    FILE *file;
    int i;


    if (document->filename[0] == '\0') {
        return -1;
    }


    file = fopen(
        document->filename,
        "w"
    );


    if (file == NULL) {
        return -1;
    }


    for (i = 0;
         i < document->line_count;
         i++) {

        if (fprintf(
                file,
                "%s\n",
                document->lines[i]
            ) < 0) {

            fclose(file);

            return -1;
        }
    }


    if (fclose(file) != 0) {
        return -1;
    }


    return 0;
}


/* =========================================================
   EXIT / CLEANUP
   ========================================================= */

void exit_editor(
    Document *document
)
{
    free_lines(
        document->lines,
        document->line_count
    );


    clear_undo_state(document);


    document->lines = NULL;
    document->line_count = 0;
    document->capacity = 0;
    document->filename[0] = '\0';
}