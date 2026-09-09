#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "infra.h"

static int increase_capacity(Document *doc) {
    int new_cap = doc->capacity * 2;
    char **temp = realloc(doc->lines, sizeof(char *) * new_cap);
    if (!temp) return -1;
    doc->lines = temp;
    doc->capacity = new_cap;
    return 0;
}

static void remove_newline(char *text) {
    text[strcspn(text, "\n")] = '\0';
}

static void free_lines(char **lines, int line_count) {
    if (!lines) return;
    for (int i = 0; i < line_count; i++) free(lines[i]);
    free(lines);
}

void clear_undo_state(Document *doc) {
    free_lines(doc->undo_lines, doc->undo_line_count);
    doc->undo_lines = NULL;
    doc->undo_line_count = doc->undo_capacity = doc->has_undo = 0;
}

void initialize_editor(Document *doc) {
    doc->lines = malloc(sizeof(char *) * INITIAL_CAPACITY);
    if (!doc->lines) {
        printf("Error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    doc->line_count = 0;
    doc->capacity = INITIAL_CAPACITY;
    doc->filename[0] = '\0';
    doc->undo_lines = NULL;
    doc->undo_line_count = doc->undo_capacity = doc->has_undo = 0;
}

int save_undo_state(Document *doc) {
    clear_undo_state(doc);
    int snap_cap = doc->capacity < INITIAL_CAPACITY ? INITIAL_CAPACITY : doc->capacity;
    char **snapshot = malloc(sizeof(char *) * snap_cap);
    if (!snapshot) return -1;

    for (int i = 0; i < doc->line_count; i++) {
        snapshot[i] = malloc(strlen(doc->lines[i]) + 1);
        if (!snapshot[i]) {
            free_lines(snapshot, i);
            return -1;
        }
        strcpy(snapshot[i], doc->lines[i]);
    }
    for (int i = doc->line_count; i < snap_cap; i++) snapshot[i] = NULL;

    doc->undo_lines = snapshot;
    doc->undo_line_count = doc->line_count;
    doc->undo_capacity = snap_cap;
    doc->has_undo = 1;
    return 0;
}

int undo_last_action(Document *doc) {
    if (!doc->has_undo) return -1;
    char **curr_lines = doc->lines;
    int curr_count = doc->line_count;

    doc->lines = doc->undo_lines;
    doc->line_count = doc->undo_line_count;
    doc->capacity = doc->undo_capacity;

    doc->undo_lines = NULL;
    doc->undo_line_count = doc->undo_capacity = doc->has_undo = 0;
    free_lines(curr_lines, curr_count);
    return 0;
}

int insert_line(Document *doc, const char *text) {
    if (!text || text[0] == '\0') return -1;
    if (doc->line_count >= doc->capacity && increase_capacity(doc) != 0) return -1;

    char *new_line = malloc(strlen(text) + 1);
    if (!new_line) return -1;
    strcpy(new_line, text);

    doc->lines[doc->line_count++] = new_line;
    return 0;
}

int delete_line(Document *doc, int line_number) {
    if (line_number < 0 || line_number >= doc->line_count) return -1;
    free(doc->lines[line_number]);

    for (int i = line_number; i < doc->line_count - 1; i++) {
        doc->lines[i] = doc->lines[i + 1];
    }
    doc->lines[--doc->line_count] = NULL;
    return 0;
}

void display_document(const Document *doc) {
    if (doc->line_count == 0) {
        printf("\nDocument is empty.\n\n");
        return;
    }
    printf("\n====================================\n           DOCUMENT\n====================================\n\n");
    for (int i = 0; i < doc->line_count; i++) printf("%d | %s\n", i, doc->lines[i]);
    printf("\n");
}

int search_document(const Document *doc, const char *query) {
    if (!query || query[0] == '\0') return -1;
    int found = 0;
    for (int i = 0; i < doc->line_count; i++) {
        if (strstr(doc->lines[i], query)) {
            printf("Found \"%s\" on line %d.\n", query, i);
            found = 1;
        }
    }
    return found;
}

static int replace_in_line(char **line, const char *find, const char *replace) {
    if (!line || !*line || !find || !replace) return -1;
    size_t f_len = strlen(find), r_len = strlen(replace);
    if (f_len == 0) return 0;

    int count = 0;
    char *pos = *line;
    while ((pos = strstr(pos, find)) != NULL) { count++; pos += f_len; }
    if (count == 0) return 0;

    size_t orig_len = strlen(*line);
    size_t new_len = orig_len + count * (r_len - f_len);
    if (new_len >= MAX_LINE_LENGTH) return -1;

    char *result = malloc(new_len + 1);
    if (!result) return -1;

    char *src = *line, *dst = result;
    while ((pos = strstr(src, find)) != NULL) {
        size_t part = pos - src;
        memcpy(dst, src, part); dst += part;
        memcpy(dst, replace, r_len); dst += r_len;
        src = pos + f_len;
    }
    strcpy(dst, src);
    free(*line);
    *line = result;
    return count;
}

int find_and_replace(Document *doc) {
    char find[MAX_LINE_LENGTH], replace[MAX_LINE_LENGTH], input[MAX_LINE_LENGTH];
    int choice, line_number, matches = 0, total_reps = 0;

    printf("\n====================================\n          FIND AND REPLACE\n====================================\n\n");
    printf("Enter word or phrase to find:\n> ");
    if (!fgets(find, sizeof(find), stdin)) return -1;
    remove_newline(find);

    if (find[0] == '\0') {
        printf("Search text cannot be empty.\n");
        return -1;
    }

    printf("\nSearching for \"%s\"...\n\n", find);
    for (int i = 0; i < doc->line_count; i++) {
        if (strstr(doc->lines[i], find)) {
            matches++;
            printf("Line %d: %s\n", i, doc->lines[i]);
        }
    }

    if (matches == 0) {
        printf("No matches found for \"%s\".\n\n", find);
        return 0;
    }

    printf("\nWhat would you like to do?\n1. Replace on one line\n2. Replace all\n3. Cancel\n\n> ");
    if (!fgets(input, sizeof(input), stdin) || sscanf(input, "%d", &choice) != 1 || (choice < 1 || choice > 3)) {
        printf("Invalid choice.\n");
        return -1;
    }
    if (choice == 3) {
        printf("Find and replace cancelled.\n\n");
        return 0;
    }

    if (choice == 1) {
        printf("\nEnter line number to replace:\n> ");
        if (!fgets(input, sizeof(input), stdin) || sscanf(input, "%d", &line_number) != 1 || line_number < 0 || line_number >= doc->line_count || !strstr(doc->lines[line_number], find)) {
            printf("Invalid line number or word not found on line.\n");
            return -1;
        }
    }

    printf("\nEnter replacement text:\n> ");
    if (!fgets(replace, sizeof(replace), stdin)) return -1;
    remove_newline(replace);

    if (choice == 1) {
        if (replace_in_line(&doc->lines[line_number], find, replace) > 0) {
            printf("\nLine %d updated successfully.\n\n", line_number);
            return 1;
        }
        printf("\nError replacing text.\n\n");
        return -1;
    }

    for (int i = 0; i < doc->line_count; i++) {
        if (strstr(doc->lines[i], find)) {
            int res = replace_in_line(&doc->lines[i], find, replace);
            if (res > 0) total_reps += res;
        }
    }
    printf("\n%d occurrence(s) replaced successfully.\n\n", total_reps);
    return total_reps > 0 ? 1 : 0;
}

void show_statistics(const Document *doc) {
    int word_count = 0;
    for (int i = 0; i < doc->line_count; i++) {
        const char *text = doc->lines[i];
        int in_word = 0;
        while (*text) {
            if (isspace((unsigned char)*text)) in_word = 0;
            else if (!in_word) { word_count++; in_word = 1; }
            text++;
        }
    }
    printf("\n====================================\n         DOCUMENT STATISTICS\n====================================\n\n");
    printf("Lines : %d\nWords : %d\n\n", doc->line_count, word_count);
}

int create_new_file(Document *doc, const char *filename) {
    if (!doc || !filename || filename[0] == '\0') return -1;
    FILE *file = fopen(filename, "r");
    if (file) { fclose(file); return 1; } // File exists

    file = fopen(filename, "w");
    if (!file || fclose(file) != 0) return -1;

    strncpy(doc->filename, filename, MAX_FILENAME_LENGTH - 1);
    doc->filename[MAX_FILENAME_LENGTH - 1] = '\0';
    clear_undo_state(doc);
    return 0;
}

int load_file(Document *doc, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return -1;

    int new_cap = INITIAL_CAPACITY, new_count = 0;
    char **new_lines = malloc(sizeof(char *) * new_cap);
    if (!new_lines) { fclose(file); return -1; }

    char buffer[MAX_LINE_LENGTH];
    while (fgets(buffer, sizeof(buffer), file)) {
        remove_newline(buffer);
        if (new_count >= new_cap) {
            new_cap *= 2;
            char **temp = realloc(new_lines, sizeof(char *) * new_cap);
            if (!temp) { fclose(file); free_lines(new_lines, new_count); return -1; }
            new_lines = temp;
        }
        new_lines[new_count] = malloc(strlen(buffer) + 1);
        if (!new_lines[new_count]) { fclose(file); free_lines(new_lines, new_count); return -1; }
        strcpy(new_lines[new_count++], buffer);
    }
    fclose(file);

    free_lines(doc->lines, doc->line_count);
    doc->lines = new_lines;
    doc->line_count = new_count;
    doc->capacity = new_cap;
    strncpy(doc->filename, filename, MAX_FILENAME_LENGTH - 1);
    doc->filename[MAX_FILENAME_LENGTH - 1] = '\0';
    clear_undo_state(doc);

    for (int i = new_count; i < new_cap; i++) doc->lines[i] = NULL;
    return 0;
}

int save_file(const Document *doc) {
    if (doc->filename[0] == '\0') return -1;
    FILE *file = fopen(doc->filename, "w");
    if (!file) return -1;

    for (int i = 0; i < doc->line_count; i++) {
        if (fprintf(file, "%s\n", doc->lines[i]) < 0) { fclose(file); return -1; }
    }
    return fclose(file) == 0 ? 0 : -1;
}

void exit_editor(Document *doc) {
    free_lines(doc->lines, doc->line_count);
    clear_undo_state(doc);
    doc->lines = NULL;
    doc->line_count = doc->capacity = 0;
    doc->filename[0] = '\0';
}