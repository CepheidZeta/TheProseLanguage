#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define CLEAR_SCREEN "\x1b[2J\x1b[1;1H"

// These macros handle text styling.
#define RESET_STYLE "\x1b[0m"
#define TITLE_STYLE "\x1b[105m\x1b[97m\x1b[1m"
#define RED_STYLE "\x1b[91m\x1b[1m"
#define GREEN_STYLE "\x1b[92m\x1b[1m"
#define YELLOW_STYLE "\x1b[93m\x1b[1m"
#define MAGENTA_STYLE "\x1b[95m\x1b[1m"
#define CYAN_STYLE "\x1b[96m\x1b[1m"
#define FILE_STYLE "\x1b[103m\x1b[30m\x1b[1m"
#define BIN_STYLE "\x1b[34m\x1b[1m"
#define HEX_STYLE "\x1b[33m\x1b[3m"

// These macros display messages.
#define INTRO_MESSAGE "\n" TITLE_STYLE " THE PROSE LANGUAGE " RESET_STYLE\
	"\n\nThis is the official loader, compiler, and viewer for "\
	MAGENTA_STYLE "The Prose Language" RESET_STYLE ".\n\nIf you have not done so already, "\
	"make sure to write your own prose code within a text (.txt) file. "\
	"Once you are done, pick an option in the menu below."\
	"\n\nIf you are new, check out the tutorial!"
#define OUTRO_MESSAGE "\n\n" TITLE_STYLE " PROGRAM EXITED " RESET_STYLE

// These macros display the menu.
#define MENU_OPTION(LABEL, OPTION) "\n" CYAN_STYLE #LABEL " " RESET_STYLE #OPTION
#define MENU(PROSE_FILE) CLEAR_SCREEN INTRO_MESSAGE "\n"\
	MENU_OPTION([T],Learn using a tutorial.)\
	MENU_OPTION([L],Load a file.)\
	MENU_OPTION([C],Show the compiled result.)\
	MENU_OPTION([M],Show the MIPS64 instructions of the loaded file.)\
	MENU_OPTION([B],Show binary and hexadecimal code conversions of the MIPS64 instructions.)\
	MENU_OPTION([E],Exit program.)\
	"\n\nCurrent file: " FILE_STYLE " " #PROSE_FILE " " RESET_STYLE

// More message-displaying macros..
#define L_OPTION_MESSAGE CLEAR_SCREEN "\n" FILE_STYLE " LOAD A FILE " RESET_STYLE\
	"\n\nEnter the name of the file containing your prose code.\n\n>> "

#define C_OPTION_MESSAGE CLEAR_SCREEN "\n" FILE_STYLE " PROSE CODE " RESET_STYLE "\n\n"
#define C_OPTION_MESSAGE_2 "\n\n" FILE_STYLE " COMPILED RESULT " RESET_STYLE "\n\n"
#define C_OPTION_MESSAGE_3 "\n\n" FILE_STYLE " END OF COMPILED RESULT " RESET_STYLE

#define M_OPTION_MESSAGE CLEAR_SCREEN "\n" FILE_STYLE  " MIPS64 INSTRUCTIONS " RESET_STYLE "\n\n"
#define M_OPTION_MESSAGE_2 "\n" FILE_STYLE  " END OF MIPS64 INSTRUCTIONS " RESET_STYLE

#define FILE_NAME_LENGTH 128
#define REDIRECT_COMMAND_LENGTH 256
#define PROSE_LINE_LENGTH 256

// These macros display tutorial information.
#define TUTORIAL_1 CLEAR_SCREEN "\n" FILE_STYLE " \"Hello World\" IN PROSE " RESET_STYLE\
	"\n\nThis is how you would write a simple \"Hello World\" program in "\
	MAGENTA_STYLE "The Prose Language" RESET_STYLE ":"\
	"\n\n\t" YELLOW_STYLE "start\n\n\tdo"\
	"\n\n\t\tshow \"Hello World\"."\
	"\n\n\t" "done." RESET_STYLE\
	"\n\nThe output should look like this:"\
	"\n\n\t" GREEN_STYLE "Hello World" RESET_STYLE
#define TUTORIAL_2 CLEAR_SCREEN "\n" FILE_STYLE " VARIABLES AND DATA TYPES IN PROSE " RESET_STYLE\
	"\n\nIn " MAGENTA_STYLE "The Prose Language" RESET_STYLE\
	", there are two data types for variables: " HEX_STYLE "character " RESET_STYLE\
	"and " HEX_STYLE "integer" RESET_STYLE "."\
	"\n\nIntegers store positive or negative whole numbers."\
	"\nCharacters store one or more letters."\
	"\n\nVariables should be defined in the " YELLOW_STYLE "start " RESET_STYLE "section."\
	"\n\n\t" YELLOW_STYLE "start"\
	"\n\n\t\tinteger a, b, c.\n\t\tcharacter n."\
	"\n\n\tdo\n\n\t\ta = 10.\n\t\tb = 20\n\t\tc = 30.\n\t\ta = a + b + c.\n\t\tn = \"Result\"."\
	"\n\t\tshow \"[n] = [a]\".\n\n\tdone." RESET_STYLE\
	"\n\nThe output should look like this:"\
	"\n\n\t" GREEN_STYLE "Result = 60" RESET_STYLE
#define TUTORIAL_3 CLEAR_SCREEN "\n" FILE_STYLE " SHOWING MESSAGES AND VARIABLES IN PROSE " RESET_STYLE\
	"\n\nIn " MAGENTA_STYLE "The Prose Language" RESET_STYLE\
	", messages and variables can be displayed using the " YELLOW_STYLE "show " RESET_STYLE "keyword."\
	"\n\nThis is how message showing is done:"\
	"\n\n\t" YELLOW_STYLE "show \"This is a message.\"" RESET_STYLE\
	"\n\nThis is how variable showing is done:"\
	"\n\n\t" YELLOW_STYLE "show var." RESET_STYLE\
	"\nOr:" YELLOW_STYLE "\n\tshow \"[var]\"." RESET_STYLE
#define TUTORIAL_4 CLEAR_SCREEN "\n" FILE_STYLE " CODE STRUCTURE IN PROSE " RESET_STYLE\
	"\n\nIn " MAGENTA_STYLE "The Prose Language" RESET_STYLE\
	", there are three structure keywords that are required:"\
	"\n\n" YELLOW_STYLE "start " RESET_STYLE "marks the beginning of the program and "\
	"is where all variables must be declared. Each line must end with a period (.)"\
	"\n\n" YELLOW_STYLE "do " RESET_STYLE "contains the execution code block where operations, "\
	"calculations, and outputs occur. Each line must end with a period (.)"\
	"\n\n" YELLOW_STYLE "done. " RESET_STYLE "marks the end of the program. Note the ending period (.)"\
	"\n\nSo overall, we have:"\
	"\n\n\t" YELLOW_STYLE "start"\
	"\n\n\t\t" BIN_STYLE "// variable declarations"\
	"\n\n\t" YELLOW_STYLE "do"\
	"\n\n\t\t" BIN_STYLE "// operations, calculations, and outputs"\
	"\n\n\t" YELLOW_STYLE "done." RESET_STYLE

/**
 * Prompts the user to return to the main menu.
 */
void prompt_menu()
{
	printf("\n\nPress X to return to the main menu.");
	while (1)
	{
		if (kbhit())
		{
			if (getch() == 'x' || getch() == 'X')
			{
				break;
			}
		}
	}
}

/**
 * Prompts the user to move to the next portion of the tutorial.
 */
void prompt_next()
{
	printf("\n\nPress N to continue.");
	fflush(stdin);
	while (1)
	{
		if (kbhit())
		{
			if (getch() == 'n' || getch() == 'N')
			{
				break;
			}
		}
	}
}

/**
 * Loads prose code from some input file,
 * and creates an auxiliary file to hold prose.exe output.
 */
void load_file( char file_name[FILE_NAME_LENGTH], FILE** file, FILE** aux )
{
	printf(L_OPTION_MESSAGE);
	scanf("%s", file_name);
	*file = fopen(file_name, "r");

	// The user is repeatedly asked to enter a file.
	// If they do not have one, then this gives them the chance to make one.
	while (*file == NULL)
	{
		printf("\n" RED_STYLE "The file was not found.");
		printf("\n" RESET_STYLE "Please try again.\n\n>> ");
		scanf("%s", file_name);
		*file = fopen(file_name, "r");
	}

	char command[REDIRECT_COMMAND_LENGTH] = ".\\prose ";

	// This runs prose.exe
	strcat(command, file_name);
	strcat(command, " > aux.txt");
	system(command);

	*aux = fopen("aux.txt", "r");
}

/**
 * Shows the compiled results of the prose code.
 */
void compile_file( FILE* file, FILE* aux )
{
	char buffer[PROSE_LINE_LENGTH];

	printf(C_OPTION_MESSAGE);

	// The prose code is shown first so that the user has a reference.
	while (fgets(buffer, sizeof(buffer), file))
	{
		if (buffer[0] == '\n')
		{
			continue;
		}

		if (
			strcmp(buffer, "start\n") == 0 ||
			strcmp(buffer, "do\n") == 0 ||
			strcmp(buffer, "done.") == 0
		)
		{
			printf(YELLOW_STYLE "\t%s" RESET_STYLE, buffer);
		}
		else
		{
			printf("\t\t%s", buffer);
		}
	}

	printf(C_OPTION_MESSAGE_2);

	// Anything other than the compiled result is ignored.
	while (fgets(buffer, sizeof(buffer), aux))
	{
		if (strcmp(buffer, "=== EDU-MIPS64 ===\n") == 0)
		{
			break;
		}

		if (buffer[0] == '\n')
		{
			continue;
		}

		printf("\t%s", buffer);
	}

	printf(RESET_STYLE C_OPTION_MESSAGE_3);

	prompt_menu();
}

/**
 * Shows all the MIPS64 instructions mapping to the compiled result.
 * Errors do not stop this from being loaded.
 */
void show_mips( FILE* aux )
{
	char buffer[PROSE_LINE_LENGTH];

	// Anything before the MIPS64 instructions is ignored.
	while (fgets(buffer, sizeof(buffer), aux))
	{
		if (strcmp(buffer, "=== EDU-MIPS64 ===\n") == 0)
		{
			break;
		}
	}

	printf(M_OPTION_MESSAGE);

	// MIPS64 instructions are formatted and displayed.
	while (fgets(buffer, sizeof(buffer), aux))
	{
		if (strcmp(buffer, "=== BINARY ===\n") == 0)
		{
			break;
		}

		if (
			strcmp(buffer, ".data\n") == 0 ||
			strcmp(buffer, ".code\n") == 0
		)
		{
			printf(YELLOW_STYLE "\t%s" RESET_STYLE, buffer);
		}
		else
		{
			printf("\t\t%s", buffer);
		}
	}

	printf(M_OPTION_MESSAGE_2);

	prompt_menu();
}

/**
 * Shows all the MIPS64 instructions as hexadecimal and binary code.
 * Errors do not stop this from being loaded.
 */
void show_hex( FILE* aux )
{
	char buffer[PROSE_LINE_LENGTH];

	// Anything before the hex and bin code is ignored.
	while (fgets(buffer, sizeof(buffer), aux))
	{
		if (strcmp(buffer, "=== BINARY ===\n") == 0)
		{
			break;
		}
	}

	printf(M_OPTION_MESSAGE);

	// The hex and bin code is formatted and displayed.
	while (fgets(buffer, sizeof(buffer), aux))
	{
		printf(HEX_STYLE "\t");
		for (int i = 0; i < strlen(buffer); i ++)
		{
			if (buffer[i] == '|')
			{
				printf(RESET_STYLE "%c" BIN_STYLE, buffer[i]);
			}
			else
			{
				printf("%c", buffer[i]);
			}
		}
		printf(RESET_STYLE);
	}

	printf(M_OPTION_MESSAGE_2);

	prompt_menu();
}

/**
 * Runs the tutorial for new users.
 */
void show_tutorial()
{
	printf(TUTORIAL_1);
	prompt_next();

	printf(TUTORIAL_2);
	prompt_next();

	printf(TUTORIAL_3);
	prompt_next();

	printf(TUTORIAL_4);
	prompt_menu();
}

int main( void )
{
	int file_found_flag = 0;
	char file_name[FILE_NAME_LENGTH] = "-";
	FILE* file = NULL;
	FILE* aux = NULL;

	printf(MENU(%s), file_name);

	// Program loop..
	while (1)
	{
		if (kbhit())
		{
			// Menu option choices are handled here.
			switch (getch())
			{
				case 't': case 'T':
					show_tutorial();
					printf(MENU(%s), file_name);
					break;
				case 'l': case 'L':
					load_file(file_name, &file, &aux);
					printf(MENU(%s), file_name);
					break;

				case 'c': case 'C':
					rewind(file);
					rewind(aux);
					compile_file(file, aux);
					printf(MENU(%s), file_name);
					break;

				case 'm': case 'M':
					rewind(aux);
					show_mips(aux);
					printf(MENU(%s), file_name);
					break;

				case 'b': case 'B':
					rewind(aux);
					show_hex(aux);
					printf(MENU(%s), file_name);
					break;

				case 'e': case 'E':
					printf(OUTRO_MESSAGE);
					if (file != NULL)
					{
						fclose(file);
					}
					if (aux != NULL)
					{
						fclose(aux);
						system("del aux.txt");
					}
					return EXIT_SUCCESS;
			}
			fflush(stdin);
		}
	}

	return EXIT_SUCCESS;
}