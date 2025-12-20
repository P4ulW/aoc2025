#ifndef ANSI_CODES_H
#define ANSI_CODES_H

// --- Style Codes ---
#define ANSI_RESET "\x1b[0m"
#define ANSI_BOLD "\x1b[1m"
#define ANSI_FAINT "\x1b[2m"
#define ANSI_ITALIC "\x1b[3m"
#define ANSI_UNDERLINE "\x1b[4m"
#define ANSI_BLINK "\x1b[5m"
#define ANSI_REVERSE "\x1b[7m"
#define ANSI_HIDDEN "\x1b[8m"

// --- Foreground (3x) Colors ---
#define ANSI_TEXT_BLACK "\x1b[30m"
#define ANSI_TEXT_RED "\x1b[31m"
#define ANSI_TEXT_GREEN "\x1b[32m"
#define ANSI_TEXT_YELLOW "\x1b[33m"
#define ANSI_TEXT_BLUE "\x1b[34m"
#define ANSI_TEXT_MAGENTA "\x1b[35m"
#define ANSI_TEXT_CYAN "\x1b[36m"
#define ANSI_TEXT_WHITE "\x1b[37m"
#define ANSI_TEXT_DEFAULT "\x1b[39m"

// --- Background (4x) Colors ---
#define ANSI_BG_BLACK "\x1b[40m"
#define ANSI_BG_RED "\x1b[41m"
#define ANSI_BG_GREEN "\x1b[42m"
#define ANSI_BG_YELLOW "\x1b[43m"
#define ANSI_BG_BLUE "\x1b[44m"
#define ANSI_BG_MAGENTA "\x1b[45m"
#define ANSI_BG_CYAN "\x1b[46m"
#define ANSI_BG_WHITE "\x1b[47m"
#define ANSI_BG_DEFAULT "\x1b[49m"

// --- Bright Foreground (9x) Colors ---
#define ANSI_TEXT_B_RED "\x1b[91m"
#define ANSI_TEXT_B_GREEN "\x1b[92m"
#define ANSI_TEXT_B_YELLOW "\x1b[93m"
#define ANSI_TEXT_B_BLUE "\x1b[94m"

// --- Bright Background (10x) Colors ---
#define ANSI_BG_B_RED "\x1b[101m"
#define ANSI_BG_B_GREEN "\x1b[102m"
#define ANSI_BG_B_YELLOW "\x1b[103m"
#define ANSI_BG_B_BLUE "\x1b[104m"

#endif  // !ANSI_CODES_H
