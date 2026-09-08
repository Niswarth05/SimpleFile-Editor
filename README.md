# SimpleFile-Editor
# Simple File Editor

A simple command-line line editor written in C.

## Features

- Load existing text files
- Create new text files
- Insert lines
- Delete lines
- Display document with line numbers
- Save files
- Search for words or phrases
- Find and replace text
- Undo the last action
- Show line and word count

## Project Structure

```text
SimpleFile-Editor/
├── main.c
├── editor.c
├── infra.h
├── Makefile
├── story.txt
├── README.md
└── .gitignore
```

## Requirements

- GCC
- Make
- Linux, macOS, WSL, or another Unix-like environment

## Compilation

```bash
make
```

Or:

```bash
gcc -Wall -Wextra -std=c11 main.c editor.c -o editor
```

## Running

```bash
./editor
```

## Commands

| Command | Description |
|---|---|
| `insert <text>` | Insert a new line |
| `delete <line>` | Delete a line |
| `display` | Display the document |
| `search <word or phrase>` | Search for text |
| `findreplace` | Find and replace text |
| `undo` | Undo the last action |
| `stats` | Show line and word count |
| `save` | Save the document |
| `help` | Show available commands |
| `exit` | Exit the editor |

## Data Structure

The editor uses a dynamically allocated array of character pointers:

```c
char **lines;
```

Each pointer stores one line of the document. The array grows dynamically when more space is required.

## Author

Developed for the C Line Editor Competition.

## License

This project is for educational and competition purposes.