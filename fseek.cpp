#include<stdio.h>
#include<stdlib.h>
#include<windows.h>

char line[1024] = { 0 };

int main()
{
	FILE* fptr = NULL;
	int   size = 0;

	fptr = fopen("C:\\Users\\lzq123218\\Desktop\\tmp.txt", "r");

	if (fptr == NULL)
	{
		
	}
	else
	{
		fseek(fptr, 0L, SEEK_END);
		size = ftell(fptr);
		fseek(fptr, 0L, SEEK_SET);
		if (size < 80)
		{
			printf("\nGOT IP FAIL!!!");
		}
		else
		{
			int   istart = 0, iend = size-1;

			fread(line, 1, size, fptr);
			while (line[istart] != '{')
			{
				istart++;
			}
			while (line[iend] != ',')
			{
				iend--;
			}




		}

		fclose(fptr);
	}

	getchar();
	return 0;
}
