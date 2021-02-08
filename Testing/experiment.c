#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	int num = 0;
	if (argc > 1) {
		num = atoi(argv[1]);
	}
	int out = num / 60.0 * 24;
	printf("%d\n", out);
	
	return 0;
}