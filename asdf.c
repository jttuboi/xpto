#include<stdio.h>
#include<stdlib.h>

int main() {

	int *x = (int *)malloc(5*sizeof(int));
	int i;
	for (i = 0; i < 5; i++)
		x[i] = i;
	printf("Test\n");
	for (i = 0; i < 5; i++)
		printf("%d\n", x[i]);
	return 0;
}
