#include <stdio.h>
#include <time.h>

int main() {
	int a = 5;
	char msg[] = "The message is %d\n";
	printf(msg,a);
	time_t t;
	time(&t);
	strftime(msg,20,"%Y_%m_%d_%H_%M_%S\n",localtime(&t));
	printf(msg);
	
	return 0;
}