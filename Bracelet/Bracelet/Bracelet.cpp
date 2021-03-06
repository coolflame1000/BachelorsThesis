// Bracelet.cpp : Defines the entry point for the console application.
//

/*ADDED: 
	std::queue<char> query_queue = std::queue<char>();
	char arduino_value[READ_LEN]; to store the chars after the endl
	that will make up our reading
	double start_time
	void tic()
	double toc()
	int queryArduinoQueue
	See Serial.cpp for ReadDataQueue
*/

#include "stdafx.h"
#include <ctime>
#include <queue>

using namespace std;

const int LEN = 4096;
const int READ_LEN = 4;
char incomingData[LEN] = "";
std::queue<char> query_queue = std::queue<char>();
char arduino_value[READ_LEN];
int dataLength = LEN - 1;
double start_time;

void tic() {
	start_time = std::clock();
}

double toc() {
	return (std::clock() - start_time) * 1000./ (double)CLOCKS_PER_SEC;
}

void flushArduino(Serial *SP) {
	while (SP->ReadData(incomingData, dataLength) != 0); 
}

int queryArduino(Serial *SP) {
	flushArduino(SP);
	while (SP->ReadData(incomingData, dataLength, 32) == 0);
	char *start = incomingData;
	while (*start++ != '\n');
	int read = strtol(start, nullptr, 10);
	return read;
}

int queryArduinoQueue(Serial *SP) {
	flushArduino(SP);
	while (SP->ReadDataQueue(&query_queue, incomingData, dataLength, 32) == 0);
	while (query_queue.front() != '\n') {query_queue.pop();}
	query_queue.pop();
	int count = 0;
	memset(arduino_value, 0, READ_LEN);
	while (query_queue.front() != '\n') {
		arduino_value[count++] = query_queue.front();
		query_queue.pop();
	}
	int read = strtol(arduino_value, nullptr, 10);
	query_queue.push(read);
	return read;
}

void defaultExecution(Serial* SP, int bentThresh, int flatThresh, int timeThresh) {
	if (SP->IsConnected()) {
		printf("Welcome to the Bracelet Console!\n");
	}

	int readResult = 0;
	int poseState = 0;
	int milliseconds = 0; //time elapsed since initial bending was detected
	
	while (SP->IsConnected()) {
		// Sleep(sleepAmount);
		readResult = queryArduinoQueue(SP);
		//std::cout << "(" << readResult << ")" << std::endl;

		{
			if (poseState == 0) { //initial state, arm is flat on table
				if (readResult > bentThresh) { //recognise arm was bent enough to trigger next state
					poseState = 1;
					tic();
				}
			}
			else if (poseState == 1) { //arm was bent, looking for second part of gesture

				milliseconds = (int) toc();

				std::cout << "Timer: " << milliseconds << std::endl;

				if (milliseconds >= timeThresh) { //took too long, return to initial state
					poseState = 2;
				}
				else if (readResult < flatThresh) { //recognise arm was flattened enough to trigger action
													//actuateMotors(full)
					std::cout << "WRRRRR\n";
					poseState = 0;
				}
			}
			else if (poseState == 2) {
				if (readResult < flatThresh) {
					poseState = 0;
				}
			}
		}
		//std::cout << "(" << readResult << ", " << poseState << ")" << std::endl;
	}

}

void findThresh(Serial* SP, int* bentThresh, int* flatThresh) {
	int readResult = 0;
	int minLength = 1;
	int maxLength = 2;

	do {
		if (maxLength <= minLength) {
			std::cout << "ERROR: please try again" << std::endl;
			std::cout << minLength << " " << maxLength << std::endl;
		}

		{
			std::cout << "Bend it yo, then press [ENTER]" << std::endl;
			while (getchar() != '\n');
			maxLength = queryArduinoQueue(SP);
		}

		{
			std::cout << "Lay arm as flat as possible, then press [ENTER]" << std::endl;
			while (getchar() != '\n');
			minLength = queryArduinoQueue(SP);
		}

	} while (maxLength <= minLength);

	std::cout << minLength << " " << maxLength << std::endl;

	*flatThresh = minLength + (maxLength - minLength) / 4;
	*bentThresh = maxLength - (maxLength - minLength) / 4;
	
}

int main() {
	Serial* SP = new Serial("\\\\.\\COM3");    // adjust as needed

	bool FIND_THRESH = true;
	if (!FIND_THRESH) {
		defaultExecution(SP, 600, 525, 4000);
	} else {
		int bentThresh, flatThresh;
		findThresh(SP, &bentThresh, &flatThresh);
		defaultExecution(SP, bentThresh, flatThresh, 4000);
	}

	return 0;
}

//std::cout << argc << " " << argv[argc - 1] << "\n";


/*
readResult = strtol(incomingData, &endPtr, 10);
int tmp = 1;
while ((tmp = strtol(endPtr, &endPtr, 10)) != 0) {
	readResult = tmp;
}
*/

// std::cout << "readResult: " << readResult << " poseState: " << poseState << " ms: " << milliseconds << "\n";
// std::cout << "(" << readResult << ")" << std::endl;

/*

else if (argc == 4) {
if (strcmp(argv[1], "-d") == 0) {
int sleepAmount = strtol(argv[2], NULL, 10);
int bentThresh;
int flatThresh;
int timeThresh = strtol(argv[3], NULL, 10);

if (sleepAmount > timeThresh) {
std::cout << "ERROR: timeThresh was smaller than sleepAmount" << std::endl;
return 0;
}

findThresh(SP, &bentThresh, &flatThresh);
defaultExecution(SP, sleepAmount, bentThresh, flatThresh, timeThresh);
}
}
else if (argc == 5) {
int sleepAmount = strtol(argv[1], NULL, 10);
int bentThresh = strtol(argv[2], NULL, 10);
int flatThresh = strtol(argv[3], NULL, 10);
int timeThresh = strtol(argv[4], NULL, 10);

if (bentThresh < flatThresh) {
std::cout << "ERROR: bentThresh was smaller than flatThresh" << std::endl;
return 0;
}
else if (sleepAmount > timeThresh) {
std::cout << "ERROR: timeThresh was smaller than sleepAmount" << std::endl;
return 0;
}

defaultExecution(SP, sleepAmount, bentThresh, flatThresh, timeThresh);

//std::cout << sleepAmount << " " << bentThresh << " " << flatThresh << " " << timeThresh << std::endl;
}

*/