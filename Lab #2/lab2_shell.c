#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h> 
#include <time.h>
#include <sys/resource.h>

int main(int argc, char* argv[])
{
    int done = 0;
    char input[80];
    long prevcsw = 0;

    while(!done){
    	printf("Enter a command:");
        fgets(input, 80, stdin);
        //If the input isn't just an "Enter" keystroke
        if (strcmp(input, "\n") != 0 ){
        	//Loops through to find how many arguments there will be so the array can be properly sized. NOTE: This included the "\n" character added by fgets()
    	    int tokenCount = 1;
        	for (int i = 0; i < strlen(input); i++){
	      		if (input[i] == ' ') {
	      			tokenCount = tokenCount + 1;
	      		}
	      		if (i == (strlen(input) - 1)){
		      		//fgets tends to place "/n" symbols at the end of the input, so that needs to be removed for the command to be understood
	      			if (strcmp(&input[i], "\n") == 0){
		      			input[i] = 0;
	      			}
	      		}
			}

			const char s[2]= " ";
			char *token;
        	token = strtok(input, s); //splits input into arguments delimited by spaces. token initially only shows the first argument

	        int inputCount = 0;
	        char *command[tokenCount];

	        //assign the tokenized input to elements in an array
	        while(token){
	      		command[inputCount] = token;
	      		token = strtok(NULL, s); //reads in the next argument. This will eventually be null
	      		inputCount = inputCount + 1;
	        }
	        //the final argument needs to be NULL so that execvp() can read the vector properly
	        command[inputCount] = NULL;
			if (strcmp(command[0], "quit") != 0){
				pid_t pid;
				int status;
				clock_t start, end;
				double cpu_used;
				start = clock();
			    if ((pid = fork()) < 0) { //create a child process
			        perror("fork failure");
			        exit(1);
			    }  else if (pid == 0){
			    	if (execvp(command[0], &command[0]) < 0) { //child process executes given command, returns
		    			perror("exec failed");
		    			exit(1);
					}
			    } else {
			    	waitpid(pid, &status, 0); //parent waits for child to finish
		    		long invcsw;
					struct rusage usage;

			    	end = clock();
			    	if(status == 0) {
			    		//ls -a -l -F
			    		cpu_used = ((double) (end - start)) / CLOCKS_PER_SEC;
			    		getrusage(RUSAGE_CHILDREN, &usage);
			    		invcsw = (usage.ru_nivcsw - prevcsw);
			    		prevcsw = prevcsw + invcsw;
			    		printf("\nUses %f seconds, %f microseconds\n", cpu_used, (cpu_used * 1000000));
			    		printf("Context switches: %li\n\n", invcsw);
			    	}
			    }

			} else {
				done = 1; 
			}
        }
        
    }

    return 0;
}