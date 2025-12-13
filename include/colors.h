#ifndef COLORS_H_
#define COLORS_H_


#define CYAN_FG    "\033[36m"
#define PURPLE_FG  "\033[95m"
#define RED_FG     "\033[91m"
#define GREEN_FG   "\033[92m"
#define WHITE_FG   "\033[97m"
#define DEFAULT_FG "\033[0m"


#define CYAN(text) CYAN_FG text DEFAULT_FG
#define PURPLE(text) PURPLE_FG text DEFAULT_FG
#define RED(text) RED_FG text DEFAULT_FG
#define GREEN(text) GREEN_FG text DEFAULT_FG
#define WHITE(text) WHITE_FG text DEFAULT_FG


#endif // COLORS_H_