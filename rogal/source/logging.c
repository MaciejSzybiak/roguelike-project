#include "shared.h"
#include <stdarg.h>

#ifdef WIN32
//Windows 10 doesn't understand ASCII color symbols
HANDLE hOutput;
#define CLR_RED		SetConsoleTextAttribute(hOutput, 0xC)
#define CLR_GREEN	SetConsoleTextAttribute(hOutput, 0xA)
#define CLR_BLUE	SetConsoleTextAttribute(hOutput, 0x9)
#define CLR_YELLOW	SetConsoleTextAttribute(hOutput, 0xE)
#define CLR_CYAN	SetConsoleTextAttribute(hOutput, 0xB)

#define CLR_RESET	SetConsoleTextAttribute(hOutput, 0x7)
#else
#define CLR_RED		printf("\033[31m")
#define CLR_GREEN	printf("\033[32m")
#define CLR_BLUE	printf("\033[34m")
#define CLR_YELLOW	printf("\033[33m")
#define CLR_CYAN	printf("\033[36m")

#define CLR_RESET	printf("\033[0m")
#endif // WIN32

#define MAX_MSG_LENGTH 256

//formats and prints debug message of the given type
void d_printf(int type, const char *format, ...) {

#ifdef _DEBUG 
	//only print in debug configuration
	char text[MAX_MSG_LENGTH];

	//printf formatted string into the buffer
	va_list ptr;
	va_start(ptr, format);

	vsnprintf(text, MAX_MSG_LENGTH, format, ptr);

	va_end(ptr);

#ifdef WIN32
	if (!hOutput) {

		//set handle to winddows console output
		hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	}
#endif

	//set message color and prefix
	switch (type)
	{
		case LOG_INFO:
			CLR_CYAN;
			printf("INFO      ");
			break;
		case LOG_WARNING:
			CLR_YELLOW;
			printf("WARNING   ");
			break;
		case LOG_ERROR:
			CLR_RED;
			printf("ERROR     ");
			break;
		default:
			CLR_RESET;
			printf("DEBUG     ");
			break;
	}

	//print the message
	printf("%s", text);

	CLR_RESET;
#endif // _DEBUG 
}

void d_spacer(void) {

#ifdef WIN32
	if (!hOutput) {

		hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	}
#endif // WIN32

	CLR_GREEN;
	printf("----------\n");
	CLR_RESET;
}

void out_of_memory_error(const char *caller) {

#ifdef WIN32
	//display a message box
	MessageBox(NULL, "malloc failed: out of memory", caller, MB_OK | MB_ICONERROR);
#else
	//print a console message
	CLR_RED;
	printf("%s", caller);
	printf(": malloc failed: out of memory\n");
	CLR_RESET;
#endif // WIN32
	exit(EXIT_FAILURE); //kill the application
}