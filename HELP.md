# Simple File Editor - Help

A simple command-line text editor written in C.

## Starting the Editor

Run:

```bash
./editor

The editor will ask for a file name.

To create a new document, type:

new

Then enter the name of the new file.

Commands
1. insert

Adds a new line to the document.

Usage:

insert <text>

Example:

insert Hello World

Output:

Line inserted successfully.
2. delete

Deletes a line from the document using its line number.

Usage:

delete <line_number>

Example:

delete 2

This removes line 2 from the document.

3. display

Displays all lines in the document with their line numbers.

Usage:

display

Example:

display

Example output:

0    |    Hello World
1    |    This is a test
2    |    Simple File Editor
4. search

Searches the document for a word or phrase and reports the matching line numbers.

Usage:

search <word or phrase>

Example:

search Hello

Example output:

Found "Hello" on line 0.
5. findreplace

Finds a word or phrase and allows it to be replaced on one line or throughout the document.

Usage:

findreplace

Example:

findreplace

Enter word or phrase to find:
> Hello

What would you like to do?

1. Replace on one line
2. Replace all
3. Cancel

The command then asks for the replacement text.

6. stats

Displays the number of lines and words in the document.

Usage:

stats

Example output:

====================================
         DOCUMENT STATISTICS
====================================

Lines : 5
Words : 18
7. undo

Undoes the most recent modifying action.

Usage:

undo

Example:

undo

Output:

Last action undone successfully.
8. save

Saves the current document to its file.

Usage:

save

Example output:

File saved successfully.
9. help

Displays the list of available commands.

Usage:

help

Example output:

Available commands:

insert <text>
delete <line_number>
display
search <word or phrase>
findreplace
stats
save
help
exit
10. exit

Exits the editor.

Usage:

exit

Example:

editor> exit