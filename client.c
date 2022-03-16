#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/msg.h>

#define MAX_TEXT 40

struct my_msg_st{
	long int my_msg_type;
	char some_text[MAX_TEXT];
};

int main()
{
	int running = 1, counter = 1, testCaseCounter = 0;
	int msgid, resultid;
	struct my_msg_st some_data, result;
	char buffer[BUFSIZ];
	char *testCases[7] = {"Insert(100)", "Insert(50)", "Delete(50)", "Average", "Median", "Min", "Sum"};
	long int msg_to_receive = 0;

// initialize 2 message queues
	msgid = msgget((key_t)1234, 0666 | IPC_CREAT);
	resultid = msgget((key_t)1235, 0666 | IPC_CREAT);

	if(msgid == -1) {
		fprintf(stderr, "msgid megget failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	if(resultid == -1) {
		fprintf(stderr, "resultid megget failed woth error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
// finish initilizing

	while(running) {

	// check if inputted too much characters
		if(testCaseCounter >= 7){
			printf("Enter some command: ");
			fgets(buffer, BUFSIZ, stdin);

			if(strlen(buffer) > 35) {
				counter = 0;
			}
			strcpy(some_data.some_text, buffer);
		} else {
			printf("%s\n", testCases[testCaseCounter]);
			strcpy(some_data.some_text, testCases[testCaseCounter]);
			testCaseCounter ++;
		}
	// finish checking
		some_data.my_msg_type = 1;
	// send the command to server
		if (counter && msgsnd(msgid, (void *)&some_data, MAX_TEXT, 0) == -1) {
			fprintf(stderr, "msgsnd failed\n");
			exit(EXIT_FAILURE);
		}
	// finish sending
	// check if the command == end ? finish loop : continue loop
		if (counter && strncmp(buffer, "end", 3) == 0) {
			running = 0;
		}
	// finish checking
	// get result from server
		if(counter && msgrcv(resultid, (void *)&result, BUFSIZ, msg_to_receive, 0) == -1) {
			fprintf(stderr, "msgrcv failed with error: %d\n", errno);
			exit(EXIT_FAILURE);
		}
	// finish receving

	// print out messages or results
		if(!running)
			printf("The program exit!\n");
		else if(counter)
			printf("%s\n", result.some_text);
		else
			printf("Please input less than 35 characters!\n");
		counter = 1;
	// finish printing
	}

// delete the message queue used to receving result from server
	if(msgctl(resultid, IPC_RMID, 0) == 1){
		fprintf(stderr, "resultid msgctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}

