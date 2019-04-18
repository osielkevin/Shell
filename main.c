/*
Name: Kevin Lopez
ID: 1001351570
*/


#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                            // so we need to define what delimits our tokens.
                            // In this case  white space
                            // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10     // Mav shell only supports five arguments


//Pid tracker Array
int pid_tracker[15];

//Pid size
int pid_size = 0;

//History Array that stores it in Array
char history[50][MAX_COMMAND_SIZE];

//History size that keeps track of the array size
int history_size = 0;

//Function that stores commands into history.
void history_copy(char *token)
{
	//If it reaches max it goes back to the print.
	if(history_size > 50)
	{
		//Resets the size to 0
		history_size = 0;
	}
	//Copies it and stores it and increases size by one
	strcpy(history[history_size],token);
	history_size++;
}

//Function that handles the signals.
static void handle_signal (int sig )
{

	/*
	Determine which of the two signals were caught and
	print an appropriate message.
	*/

	switch( sig )
	{
		case SIGINT:
			//printf("Caught a SIGINT\n");
			break;

		case SIGTSTP:
			//printf("Caught a SIGTSTP\n");
			break;

		default:
			//  printf("Unable to determine the signal\n");
			break;

	}

}

//Function that does commands / prints error message.
void do_commands(char **token)
{
	//Creates fork
	pid_t child_pid = fork();
	int status;

	//If its the child then perform the commands ls cd mkdir.
	if( child_pid == 0 )
	{
		//Execvp sends the token[0] like an example would be token[0] = "cd"
		//The second parameter is whatever options they add to it like -ra
		//Also it dynamically finds the pathway
		execvp(token[0],token);
		//If an error is found then print an error message
		printf("%s: command not found \n",token[0]);
		exit( EXIT_SUCCESS );
	}

	//If its the parent then wait and also store the PID into the Array.
	else if(child_pid > 0)
	{
		//If it goes to 15 then go to the top of the array and override it with new
		if(pid_size > 15)
		{
			pid_size = 0;
		}
		//Overriding process of the pid array
		pid_tracker[pid_size] = child_pid;
		pid_size++;
	}
	//Tells the parent to wait
	waitpid( child_pid, &status, 0 );
}

//function that performs change directory
void do_cd(char **token)
{
	//Changes directory
	chdir(token[1]);
}

/*--------Main Starts Here---------*/
int main()
{
	//Sets the history size to 0
	history_size =0;
	struct sigaction act;
	/*
	Zero out the sigaction struct
	*/
	memset (&act, '\0', sizeof(act));
	/*
	Set the handler to use the function handle_signal()
	*/
	act.sa_handler = &handle_signal;
	/*
	Install the handler for SIGINT and SIGTSTP and check the
	return value.
	*/
	if (sigaction(SIGINT , &act, NULL) < 0)
	{
		perror ("sigaction: ");
		return 1;
	}

	if (sigaction(SIGTSTP , &act, NULL) < 0)
	{
		perror ("sigaction: ");
		return 1;
	}
	char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

	while( 1 )
	{
		// Print out the msh prompt
		printf ("msh> ");
		// Read the command from the commandline.  The
		// maximum command that will be read is MAX_COMMAND_SIZE
		// This while command will wait here until the user
		// inputs something since fgets returns NULL when there
		// is no input
		while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );
		/* Parse input */

		char *token[MAX_NUM_ARGUMENTS];
		int   token_count = 0;
		// Pointer to point to the token
		// parsed by strsep
		char *arg_ptr;
		char *working_str  = strdup( cmd_str );
		// we are going to move the working_str pointer so
		// keep track of its original value so we can deallocate
		// the correct amount at the end
		char *working_root = working_str;
		// Tokenize the input stringswith whitespace used as the delimiter
		while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) &&
        (token_count<MAX_NUM_ARGUMENTS))
		{
			token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
			if( strlen( token[token_count] ) == 0 )
			{
				token[token_count] = NULL;
			}
			token_count++;
		}
		//If user enters blank it will reset itself back to the next iteration of the loop.
		if(token[0] == '\0')
		{
			continue;
		}

		//This loop will contain the choices that users will go to.
		//Needed this for the ! function to work
		while (1)
		{
			//If they press cd then it will perform cd
			if (strcmp(token[0], "cd") == 0)
			{
				//Change directory and copy it to history array break afterwards
				do_cd(token);
				history_copy(cmd_str);
				break;
			}
			//Checks to see if it meets the requirement for listpids
			else if (strcmp(token[0], "listpids") == 0 && token_count == 2)
			{
				int i = 0;
				//Copies the command
				history_copy(cmd_str);
				//Prints all the pids by the format specified
				for (i = 0; i < pid_size; i++)
				{
					//Format : # and the pid #
					printf("%d: %d\n", i, pid_tracker[i]);
				}
				break;
			}
			//If history requirement met then go here
			else if (strcmp(token[0], "history") == 0 && token_count == 2)
			{
				int i = 0;
				//Copies the command
				history_copy(cmd_str);
				//Prints the format specified
				for (i = 0; i < history_size; i++)
				{
					printf("%d: %s", i, history[i]);
				}
				break;
			}
			//If user types either then program will exit
			else if ((strcmp(token[0], "quit") == 0 && token_count == 2) || (strcmp(token[0], "exit") == 0 && token_count == 2))
			{
				exit(EXIT_SUCCESS);
			}
			//If user types ! it will proof check and see it should go here
			else if (token[0][0] == '!')
			{
				//converts the ascii into an int for further evaluation
				int x = atoi(&token[0][1]);
				//duplicates this for further string manipulation
				char *copy = strdup(history[x]);
				//History must be greater than 0 the number must be between 0-15
				//Numbered also entered must less than the size of the history array
				if (history_size > 0 && x >= 0 && x <= 15 && x < history_size)
				{
					//resets token count so it can fit to the beginning of token array
					token_count = 0;
					//Code from your sample of string manipulation
					while (((arg_ptr = strsep(&copy, WHITESPACE)) != NULL) &&
						(token_count < MAX_NUM_ARGUMENTS))
					{
						token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
						if (strlen(token[token_count]) == 0)
						{
							token[token_count] = NULL;
						}
						token_count++;
					}
					//Goes back to while(1) to execute the rule
					continue;
				}
				//If the requirements not met then it will print a generic error message and copy and break
				else
				{
					//Print statement
					printf("Error Invalid Entry must be within 0 - 15 and size must be greater than 0\n");
					//Store the garbage
					history_copy(cmd_str);
					break;
				}
			}
			//Sees to perform bg
			else if (strcmp(token[0], "bg") == 0 && token_count == 2)
			{
				//Sends sigcontinue to the last pid that we stored in the pid array
				//That was suspended so we can run it in the background
				kill(pid_tracker[pid_size - 1], SIGCONT);
				//Store command to array of char
				history_copy(cmd_str);
				break;
			}
			else
			{
				//goes to function and performs the commands
				do_commands(token);
				//stores the commands
				history_copy(cmd_str);
				break;
			}
		}
		free( working_root );
	}
	return 0;
}
