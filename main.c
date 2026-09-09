#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "infra.h"

int main(void) {
    Document document;
    char input[MAX_LINE_LENGTH];

    initialize_editor(&document);
    printf("====================================\n         SIMPLE LINE EDITOR\n====================================\n\n");
    printf("Enter file name to load:\n(Type 'new' to create a new document)\n\n> ");

    if (!fgets(input, sizeof(input), stdin)) {
        printf("Error reading input.\n");
        exit_editor(&document);
        return 1;
    }
    input[strcspn(input, "\n")] = '\0';

    if (strcmp(input, "new") == 0) {
        char filename[MAX_FILENAME_LENGTH];
        printf("\nEnter new file name:\n> ");
        if (!fgets(filename, sizeof(filename), stdin)) { exit_editor(&document); return 1; }
        filename[strcspn(filename, "\n")] = '\0';

        if (filename[0] == '\0') { printf("Error: Filename cannot be empty.\n"); exit_editor(&document); return 1; }
        int result = create_new_file(&document, filename);
        if (result == 1) { printf("\nError: File '%s' already exists.\n", filename); exit_editor(&document); return 1; }
        if (result != 0) { printf("\nError: Could not create file '%s'.\n", filename); exit_editor(&document); return 1; }
        printf("\nNew file '%s' created successfully.\n\n", document.filename);
    } else {
        if (load_file(&document, input) != 0) {
            printf("\nError: Could not load '%s'.\n", input);
            exit_editor(&document);
            return 1;
        }
        printf("\nFile loaded successfully.\n%d lines loaded.\n\n", document.line_count);
    }

    printf("Type 'help' to view available commands.\n\n");

    while (1) {
        printf("editor> ");
        if (!fgets(input, sizeof(input), stdin)) { printf("\n"); break; }
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0) break;

        if (strcmp(input, "display") == 0) { display_document(&document); continue; }
        
        if (strcmp(input, "save") == 0) {
            printf(save_file(&document) == 0 ? "File saved successfully.\n" : "Error saving file.\n");
            continue;
        }

        if (strncmp(input, "insert ", 7) == 0) {
            if (save_undo_state(&document) != 0) { printf("Error: Could not prepare undo.\n"); continue; }
            if (insert_line(&document, input + 7) == 0) printf("Line inserted successfully.\n");
            else { clear_undo_state(&document); printf("Error inserting line.\n"); }
            continue;
        }

        if (strncmp(input, "delete ", 7) == 0) {
            int line_number;
            if (sscanf(input + 7, "%d", &line_number) == 1) {
                if (line_number < 0 || line_number >= document.line_count) { printf("Invalid line number.\n"); continue; }
                if (save_undo_state(&document) != 0) { printf("Error: Could not prepare undo.\n"); continue; }
                if (delete_line(&document, line_number) == 0) printf("Line %d deleted successfully.\n", line_number);
                else { clear_undo_state(&document); printf("Error deleting line.\n"); }
            } else printf("Invalid delete command.\n");
            continue;
        }

        if (strncmp(input, "search ", 7) == 0) {
            int result = search_document(&document, input + 7);
            if (result == 0) printf("No matches found for \"%s\".\n", input + 7);
            else if (result == -1) printf("Usage: search <word or phrase>\n");
            continue;
        }

        if (strcmp(input, "findreplace") == 0) {
            if (save_undo_state(&document) != 0) { printf("Error: Could not prepare undo.\n"); continue; }
            if (find_and_replace(&document) != 1) clear_undo_state(&document);
            continue;
        }

        if (strcmp(input, "undo") == 0) {
            printf(undo_last_action(&document) == 0 ? "Last action undone successfully.\n" : "Nothing to undo.\n");
            continue;
        }

        if (strcmp(input, "stats") == 0) { show_statistics(&document); continue; }

        if (strcmp(input, "help") == 0) {
            printf("\n========================================\n             HELP - COMMANDS\n========================================\n\n");
            printf("DOCUMENT COMMANDS\n----------------------------------------\ninsert <text>       Insert a new line\ndelete <line>       Delete a line\ndisplay             Display the document\n");
            printf("\nSEARCH & EDITING\n----------------------------------------\nsearch <word>       Search for a word/phrase\nfindreplace         Find and replace text\nundo                Undo the last action\n");
            printf("\nDOCUMENT INFO\n----------------------------------------\nstats               Show line & word count\nsave                Save the document\n");
            printf("\nGENERAL\n----------------------------------------\nhelp                Show this help menu\nexit                Exit the editor\n========================================\n\n");
            continue;
        }

        if (input[0] != '\0') printf("Unknown command.\n");
    }

    exit_editor(&document);
    printf("\nExiting editor...\n");
    return 0;
}