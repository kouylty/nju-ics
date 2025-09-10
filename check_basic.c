#include <stdio.h>
#include <stddef.h>

int main()
{
	printf("size of size_t: %zu\n",sizeof(size_t));
	int x = -1;
	printf("%d\n", x>>2);
	float a = (float)0x00000001, b = (float)0x7f000001;
	printf("%.12f %.12f %.12f\n", a, b, a*b);

	return 0;
}
