#include <stdio.h>
#include <windows.h>

void add(int a, int b)
{
	printf("add = %d\n", a+b);
}

void sub(int a, int b)
{
	printf("sub = %d\n", a-b);
}

int main(void)
{
	// 記得要先關防毒

	int num = 1;

	void(*fp)(int, int) = &add;

	printf("num=%p, add()=%p, sub()=%p, fp()=%p\n", &num, &add, &sub, &fp);

	while (1)
	{
		Sleep(3000);
		printf("num = %d\n", num);
		fp(num, num*2);
		num++;
	}	

	return 0;
}