#include<Windows.h>
#include<stdio.h>
#include<stdlib.h>

#pragma warning(disable:4996)
static const GUID  MineGUID = { 0x8ccb4fb6, 0x5328, 0x4b18,{ 0x85, 0x68, 0x89, 0x82, 0x88, 0xb9, 0x85, 0x18 } };

int main(int c,char* a[])
{
	char bufsoft[1024] = {0};
	char bufhard[1024] = {0};
	char Reparse[1024] = {0};
	int  len = strlen(a[1]);

	BOOLEAN						ret;
	HANDLE						hFile;
	DWORD						retv = 0, LError = 0;
	PREPARSE_GUID_DATA_BUFFER	RGDB;


	strcpy(bufhard, a[1]);
	strcat(bufhard, "hard");
	strcpy(bufsoft, a[1]);
	strcat(bufsoft, "soft");
	strcpy(Reparse, a[1]);
	strcat(Reparse, "Rpar");


	if (a[1] == NULL)
	{
		printf("\nFile name is empty!");
	}
	ret = CreateSymbolicLinkA(bufsoft, a[1], 0);
	if (ret != 0)
	{
		ret = CreateHardLinkA(bufhard, a[1], NULL);
		if (ret != 0)
		{
			printf("\nTwo Links ok...");
		}
		else
		{
			printf("\nCreateHardLinkA fails %d...",GetLastError());

			return 0;
		}
	}
	else
	{
		printf("\nCreateSymbolicLinkA fails %d...", GetLastError());

		return 0;
	}

	hFile = CreateFileA(Reparse, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		LError = GetLastError();
		printf("\nCreateFile Error %d\n", GetLastError());
	}
	else
	{
		{
			BY_HANDLE_FILE_INFORMATION  Finfo;
			HANDLE						hFile;

			hFile = CreateFileA(a[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			GetFileInformationByHandle(hFile, &Finfo);
			printf("\nRealFile id high-low:%u-%u",Finfo.nFileIndexHigh, Finfo.nFileIndexLow);

			CloseHandle(hFile);
		}
		RGDB = (REPARSE_GUID_DATA_BUFFER*)malloc(REPARSE_GUID_DATA_BUFFER_HEADER_SIZE + 0x80);
		memset(RGDB, 0, REPARSE_GUID_DATA_BUFFER_HEADER_SIZE + 0x80);

		RGDB->ReparseGuid = MineGUID;

		RGDB->ReparseTag = 0x00123218;
		
		memcpy(RGDB->GenericReparseBuffer.DataBuffer, a[1], sizeof(a[1]));

		RGDB->ReparseDataLength = sizeof(a[1]);

		if (!DeviceIoControl(hFile, FSCTL_SET_REPARSE_POINT, RGDB, REPARSE_GUID_DATA_BUFFER_HEADER_SIZE + RGDB->ReparseDataLength, NULL, 0, &retv, 0))
		{
			LError = GetLastError();
			printf("\nDeviceIoControl Error: %d\n", LError);
		}

		free(RGDB);
		CloseHandle(hFile);
	}
	{
		{
			BY_HANDLE_FILE_INFORMATION  Finfo;
			HANDLE						hFile;

			hFile = CreateFileA(bufhard, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			GetFileInformationByHandle(hFile, &Finfo);
			printf("\nHardlink id high-low:%u-%u", Finfo.nFileIndexHigh, Finfo.nFileIndexLow);
			CloseHandle(hFile);
			hFile = CreateFileA(bufsoft, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			GetFileInformationByHandle(hFile, &Finfo);
			printf("\nSoftlink id high-low:%u-%u", Finfo.nFileIndexHigh, Finfo.nFileIndexLow);
			CloseHandle(hFile);
			hFile = CreateFileA(Reparse, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			GetFileInformationByHandle(hFile, &Finfo);
			printf("\nReparse id high-low:%u-%u", Finfo.nFileIndexHigh, Finfo.nFileIndexLow);
			CloseHandle(hFile);


		}
	}
	


	return 0;
}
