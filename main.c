#include <stdio.h>
#include <stdlib.h>

int main(){
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

 // Now print the tokenized input as a debug check
 // \TODO Remove this code and replace with your shell functionality

 int token_index  = 0;
 for( token_index = 0; token_index < token_count; token_index ++ )
 {
   printf("token[%d] = %s\n", token_index, token[token_index] );
 }

 free( working_root );

}
return 0;
}
