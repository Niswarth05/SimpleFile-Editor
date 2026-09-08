#include <stdio.h>
#include <string.h>

#include "infra.h"


int main(void)
{
    Document document;
    char input[MAX_LINE_LENGTH];


    initialize_editor(&document);


    printf("====================================\n");
    printf("         SIMPLE LINE EDITOR\n");
    printf("====================================\n\n");


    printf("Enter file name to load:\n");
    printf("(Type 'new' to create a new document)\n\n");

    printf("> ");


    if (fgets(
            input,
            sizeof(input),
            stdin
        ) == NULL) {

        printf(
            "Error reading input.\n"
        );

        exit_editor(&document);

        return 1;
    }


    input[
        strcspn(input, "\n")
    ] = '\0';


    /*
     * Create a new document.
     */
    if (strcmp(input, "new") == 0) {

        char filename[MAX_FILENAME_LENGTH];
        int result;


        printf(
            "\nEnter new file name:\n"
        );

        printf("> ");


        if (fgets(
                filename,
                sizeof(filename),
                stdin
            ) == NULL) {

            printf(
                "Error reading filename.\n"
            );

            exit_editor(&document);

            return 1;
        }


        filename[
            strcspn(filename, "\n")
        ] = '\0';


        if (filename[0] == '\0') {

            printf(
                "Error: Filename cannot be empty.\n"
            );

            exit_editor(&document);

            return 1;
        }


        result = create_new_file(
            &document,
            filename
        );


        if (result == 1) {

            printf(
                "\nError: File '%s' already exists.\n",
                filename
            );

            exit_editor(&document);

            return 1;
        }


        if (result != 0) {

            printf(
                "\nError: Could not create file '%s'.\n",
                filename
            );

            exit_editor(&document);

            return 1;
        }


        printf(
            "\nNew file '%s' created successfully.\n\n",
            document.filename
        );
    }


    /*
     * Load an existing file.
     */
    else {

        if (load_file(
                &document,
                input
            ) != 0) {

            printf(
                "\nError: Could not load '%s'.\n",
                input
            );

            exit_editor(&document);

            return 1;
        }


        printf(
            "\nFile loaded successfully.\n"
        );

        printf(
            "%d lines loaded.\n\n",
            document.line_count
        );
    }


    printf(
        "Type 'help' to view available commands.\n\n"
    );


    /*
     * Main editor loop.
     */
    while (1) {

        printf("editor> ");


        if (fgets(
                input,
                sizeof(input),
                stdin
            ) == NULL) {

            printf("\n");

            break;
        }


        input[
            strcspn(input, "\n")
        ] = '\0';


        /* =========================
           EXIT
           ========================= */

        if (strcmp(
                input,
                "exit"
            ) == 0) {

            break;
        }


        /* =========================
           DISPLAY
           ========================= */

        if (strcmp(
                input,
                "display"
            ) == 0) {

            display_document(
                &document
            );

            continue;
        }


        /* =========================
           SAVE
           ========================= */

        if (strcmp(
                input,
                "save"
            ) == 0) {

            if (save_file(
                    &document
                ) == 0) {

                printf(
                    "File saved successfully.\n"
                );

            } else {

                printf(
                    "Error saving file.\n"
                );
            }

            continue;
        }


        /* =========================
           INSERT
           ========================= */

        if (strncmp(
                input,
                "insert ",
                7
            ) == 0) {

            if (save_undo_state(
                    &document
                ) != 0) {

                printf(
                    "Error: Could not prepare undo.\n"
                );

                continue;
            }


            if (insert_line(
                    &document,
                    input + 7
                ) == 0) {

                printf(
                    "Line inserted successfully.\n"
                );

            } else {

                clear_undo_state(
                    &document
                );

                printf(
                    "Error inserting line.\n"
                );
            }

            continue;
        }


        /* =========================
           DELETE
           ========================= */

        if (strncmp(
                input,
                "delete ",
                7
            ) == 0) {

            int line_number;


            if (sscanf(
                    input + 7,
                    "%d",
                    &line_number
                ) == 1) {


                if (line_number < 0 ||
                    line_number >=
                        document.line_count) {

                    printf(
                        "Invalid line number.\n"
                    );

                    continue;
                }


                if (save_undo_state(
                        &document
                    ) != 0) {

                    printf(
                        "Error: Could not prepare undo.\n"
                    );

                    continue;
                }


                if (delete_line(
                        &document,
                        line_number
                    ) == 0) {

                    printf(
                        "Line %d deleted successfully.\n",
                        line_number
                    );

                } else {

                    clear_undo_state(
                        &document
                    );

                    printf(
                        "Error deleting line.\n"
                    );
                }

            } else {

                printf(
                    "Invalid delete command.\n"
                );
            }

            continue;
        }


        /* =========================
           SEARCH
           ========================= */

        if (strncmp(
                input,
                "search ",
                7
            ) == 0) {

            const char *query;
            int result;


            query = input + 7;


            result = search_document(
                &document,
                query
            );


            if (result == 0) {

                printf(
                    "No matches found for \"%s\".\n",
                    query
                );

            } else if (result == -1) {

                printf(
                    "Usage: search <word or phrase>\n"
                );
            }

            continue;
        }


        /* =========================
           FIND AND REPLACE
           ========================= */

        if (strcmp(
                input,
                "findreplace"
            ) == 0) {

            int result;


            if (save_undo_state(
                    &document
                ) != 0) {

                printf(
                    "Error: Could not prepare undo.\n"
                );

                continue;
            }


            result = find_and_replace(
                &document
            );


            /*
             * Keep undo only when
             * something was actually changed.
             */
            if (result != 1) {

                clear_undo_state(
                    &document
                );
            }


            continue;
        }


        /* =========================
           UNDO
           ========================= */

        if (strcmp(
                input,
                "undo"
            ) == 0) {

            if (undo_last_action(
                    &document
                ) == 0) {

                printf(
                    "Last action undone successfully.\n"
                );

            } else {

                printf(
                    "Nothing to undo.\n"
                );
            }

            continue;
        }


        /* =========================
           STATISTICS
           ========================= */

        if (strcmp(
                input,
                "stats"
            ) == 0) {

            show_statistics(
                &document
            );

            continue;
        }


        /* =========================
           HELP
           ========================= */

        if (strcmp(
                input,
                "help"
            ) == 0) {

            printf("\n");
            printf("========================================\n");
            printf("             HELP - COMMANDS\n");
            printf("========================================\n\n");


            printf("DOCUMENT COMMANDS\n");
            printf("----------------------------------------\n");

            printf(
                "insert <text>       Insert a new line\n"
            );

            printf(
                "delete <line>       Delete a line\n"
            );

            printf(
                "display             Display the document\n"
            );


            printf("\nSEARCH & EDITING\n");
            printf("----------------------------------------\n");

            printf(
                "search <word>       Search for a word/phrase\n"
            );

            printf(
                "findreplace         Find and replace text\n"
            );

            printf(
                "undo                Undo the last action\n"
            );


            printf("\nDOCUMENT INFO\n");
            printf("----------------------------------------\n");

            printf(
                "stats               Show line & word count\n"
            );

            printf(
                "save                Save the document\n"
            );


            printf("\nGENERAL\n");
            printf("----------------------------------------\n");

            printf(
                "help                Show this help menu\n"
            );

            printf(
                "exit                Exit the editor\n"
            );


            printf(
                "\n========================================\n\n"
            );

            continue;
        }


        /* =========================
           UNKNOWN COMMAND
           ========================= */

        if (input[0] != '\0') {

            printf(
                "Unknown command.\n"
            );
        }
    }


    /*
     * Free all allocated memory.
     */
    exit_editor(
        &document
    );


    printf(
        "\nExiting editor...\n"
    );


    return 0;
}