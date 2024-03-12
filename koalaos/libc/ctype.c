#include "ctype.h"

int isdigit(int c) {
	return ((unsigned)c-'0') < 10;
}

int isspace(int c) {
	return c == ' ' || (unsigned)c-'\t' < 5;
}