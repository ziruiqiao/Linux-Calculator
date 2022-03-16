#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>

#include <sys/msg.h>

#define MAX_TEXT 40

struct my_msg_st{
	long int my_msg_type;
	char some_text[BUFSIZ];
};

void getTime(struct timeval start, struct timeval end, int timeResult[], char *command);
char *DoCommand(char* command, int timeResult[]);
int isdigitstr(char *str);
int Remove(float num);

float numbers[100];
int size = 0;
float accuracy = 0.000001;


int main()
{
	int running = 1;
	int msgid, resultid;
	int timeResult[6] = {0, 0, 0, 0, 0, 0};
	struct my_msg_st some_data, result;
	long int msg_to_receive = 0;
	char* command;

// initialize 2 message queues
	msgid = msgget((key_t)1234, 0666 | IPC_CREAT);
	resultid = msgget((key_t)1235, 0666 | IPC_CREAT);

	if(msgid == -1) {
		fprintf(stderr, "msgid megget failed woth error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	if(resultid == -1) {
		fprintf(stderr, "resultid megget failed woth error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
// finish initializing

	while(running) {
	// recieve command from client
		if(msgrcv(msgid, (void *)&some_data, BUFSIZ, msg_to_receive, 0) == -1) {
			fprintf(stderr, "msgrcv failed with error: %d\n", errno);
			exit(EXIT_FAILURE);
		}
	// finish receiving
		char* temp;
	// calculate according to the command
		temp = DoCommand(some_data.some_text, timeResult);
	//finish calculating

		strcpy(result.some_text, temp);
		result.my_msg_type = 1;
		free(temp);

	// try to print Average Time if all operations are used at least once
		int sumOfTime = 0;
		for(int i=0; i<6; i++){
			if(timeResult[i] == 0){
				sumOfTime = 0;
				break;
			}
			sumOfTime += timeResult[i];
		}
		if(sumOfTime != 0){
			printf("\nAverage running time of the 6 commands= %.2f micro seconds\n\n", sumOfTime*1.0/6);
		}
	// finish printing

	// send result back through another message queue
		if(msgsnd(resultid, (void *)&result, MAX_TEXT, 0)==-1) {
			fprintf(stderr, "resultid msgsnd failed\n");
			exit(EXIT_FAILURE);
		}
	// finish sending

	// if command == end, end the program.
		if(strncmp(some_data.some_text, "end", 3) == 0) {
			running = 0;
		}
	}

// delete the message queue used to receive commands
	if(msgctl(msgid, IPC_RMID, 0) == -1){
		fprintf(stderr, "msgid msgctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}

/*
*	calculate according to the input command
*/
char* DoCommand(char* command, int timeResult[])
{
	char *result, *temp, *message;
        char *errorMessage;
	float num;
	struct timeval start, end;

	gettimeofday(&start, NULL);
	result = (char *)malloc(35);
	errorMessage = (char *)malloc(30);
	strcpy(errorMessage, "Please input a right command");
	// if command is Insert(x) or Delete(x)
	if(strlen(command) >= 9) {
		if (strncmp(command, "Insert(", 7)==0 || strncmp(command, "Delete(", 7)==0){
			temp = strtok(command, "(");
			temp = strtok(NULL, "(");
			if(strlen(temp) == 0) {
				getTime(start, end, timeResult, command);
				return errorMessage;
			}
			temp = strtok(temp, ")");
			if(strlen(temp) == 0) {
				getTime(start, end, timeResult, command);
				return errorMessage;
			}
			if(isdigitstr(temp) == 0) {
				getTime(start, end, timeResult, command);
				return errorMessage;
			}
			num = atof(temp);
			// insert number and arrange the list of number from small to large
			if(strncmp(command, "Insert", 6) == 0){
				numbers[size] = num;
				size++;
				for(int i=0; i<size; i++){
					for(int j=i; j<size; j++){
						if(numbers[i] > numbers[j]){
							num = numbers[i];
							numbers[i] = numbers[j];
							numbers[j] = num;
						}
					}
				}
				message = "number list added ";
			} else {
				if(Remove(num)) 
					message = "number list removed ";
				else
					message = "list doesn't have ";
			}
			strcat(result, message);
			strcat(result, temp);
			getTime(start, end, timeResult, command);
			return result;
		}
	}
	// if command is Average
	if(strlen(command) >= 7) {
		if(strncmp(command, "Average", 7) == 0){
			char temp[10];
			for(int i=0; i<size; i++){
				num += numbers[i];
			}
			num = num/size;
			message = "The average is ";
			if(fabs(num - (int)num) < accuracy) num = (int)num;
			sprintf(temp, "%.2f", num);
			strcat(result, message);
			strcat(result, temp);
			getTime(start, end, timeResult, command);
			return result;
		}
	}
	// if command is Median
	if(strlen(command) >= 6) {
		if(strncmp(command, "Median", 6) == 0){
			char temp[10];
			if(size %2 == 1) num = numbers[(size - 1)/2];
			else num = (numbers[size/2] + numbers[size/2 - 1])/2;
			if(fabs(num - (int)num) < accuracy) num = (int)num;
			sprintf(temp, "%.2f", num);
			message = "The median is ";
			strcat(result, message);
			strcat(result, temp);
			getTime(start, end, timeResult, command);
			return result;
		}
	}
	// if command is Sum or Min
	if(strlen(command) >= 3) {
		if(strncmp(command, "Min", 3) == 0){
			char temp[10];
			num = numbers[0];	
			if(fabs(num - (int)num) < accuracy) num = (int)num;
			sprintf(temp, "%.2f", num);
			message = "The min is ";
			strcat(result, message);
			strcat(result, temp);
			getTime(start, end, timeResult, command);
			return result;
		}
		if(strncmp(command, "Sum", 3) == 0){
			char temp[10];
			for(int i=0; i<size; i++){
				num += numbers[i];
			}
			if(fabs(num - (int)num) < accuracy) num = (int)num;
			sprintf(temp, "%.2f", num);
			message = "The Sum is ";
			strcat(result, message);
			strcat(result, temp);
			getTime(start, end, timeResult, command);
			return result;
		}
		if(strncmp(command, "end", 3) == 0){
			char temp[10];
			message = "Server closes";
			strcat(result, message);
			return result;
		}
	}
	getTime(start, end, timeResult, command);
	return errorMessage;
}

/*
*	get the time of operation and load the result to the corresponding time counter
*/
void getTime(struct timeval start, struct timeval end, int timeResult[], char *command){	
	int result;
	gettimeofday(&end, NULL);
	result = (end.tv_sec * 1000000+end.tv_usec) - (start.tv_sec * 1000000+start.tv_usec);

	if(strncmp(command, "Insert", 6)==0){
		if(timeResult[0] != 0)timeResult[0] = (int)(result + timeResult[0])/2.0;
		else timeResult[0] = result;
		printf("Command Insert");
	}else if(strncmp(command, "Delete", 6)==0){
		if(timeResult[1] != 0)timeResult[1] = (int)(result + timeResult[1])/2.0;
		else timeResult[1] = result;
		printf("Command Delete");
	}else if(strncmp(command, "Average", 7)==0){
		if(timeResult[2] != 0)timeResult[2] = (int)(result + timeResult[2])/2.0;
		else timeResult[2] = result;
		printf("Command Average");
	}else if(strncmp(command, "Median", 6)==0){
		if(timeResult[3] != 0)timeResult[3] = (int)(result + timeResult[3])/2.0;
		else timeResult[3] = result;
		printf("Command Median");
	}else if(strncmp(command, "Min", 3)==0){
		if(timeResult[4] != 0)timeResult[4] = (int)(result + timeResult[4])/2.0;
		else timeResult[4] = result;
		printf("Command Min");
	}else if(strncmp(command, "Sum", 3)==0){
		if(timeResult[5] != 0)timeResult[5] = (int)(result + timeResult[5])/2.0;
		else timeResult[5] = result;
		printf("Command Sum");
	}else {
		printf("Wrong commands");
	}
	printf(" took %1d micro seconds\n", result);
}

/*
*	(check if the given char[] is a number) ? 1:0
*/
int isdigitstr(char *str)
{
	int len = strlen(str), i=0, counter=0;

	for(i; i<len; i++){
		if(str[i]>'9' || str[i]<'0' && str[i]!='.'){
			return 0;
		}
		if(str[i] == '.'){
			if(i == 0 || i == len-1){
				return 0;
			}
			counter ++;
		}
	}
	if(counter <= 1) return 1;
	else return 0;
}

/*
*	remove the given number from the calculator, 
*	if remove successfully, return 1
*	else return 0
*/
int Remove(float num){
	int counter = 0;
	for(int i=0; i<size; i++){
		if(fabs(numbers[i]-num) < accuracy){
			numbers[i] = 0.0;
			counter++;
		}
	}
	for(int k=0; k<size; k++){
		for(int i=0; i<size; i++){
			if(fabs(numbers[i] - 0.0) < accuracy){
				for(int j=i; j<size; j++){
				       numbers[j] = numbers[j+1];
				}
				numbers[size - 1] = 0.0;
			}
		}
	}
	size -= counter;
	if(counter == 0) return 0;
	return 1;
}
