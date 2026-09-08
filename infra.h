#ifndef INFRA_H
#define INFRA_H

#define INITIAL_CAPACITY 10
#define MAX_FILENAME_LENGTH 256
#define MAX_LINE_LENGTH 1024

typedef struct {
    char **lines;
    int line_count;
    int capacity;
    char filename[MAX_FILENAME_LENGTH];

    /* Undo support */
    char **undo_lines;
    int undo_line_count;
    int undo_capacity;
    int has_undo;

} Document;


/* Document initialization and cleanup */

void initialize_editor(Document *document);
void exit_editor(Document *document);


/* File operations */

int load_file(Document *document, const char *filename);
int save_file(const Document *document);
int create_new_file(Document *document, const char *filename);


/* Document operations */

int insert_line(Document *document, const char *text);
int delete_line(Document *document, int line_number);
void display_document(const Document *document);


/* Search */

int search_document(const Document *document, const char *query);


/* Find and Replace */

int find_and_replace(Document *document);


/* Statistics */

void show_statistics(const Document *document);


/* Undo */

int save_undo_state(Document *document);
int undo_last_action(Document *document);
void clear_undo_state(Document *document);

#endif