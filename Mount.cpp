#include<stdio.h>
#include<stdlib.h>
#include<windows.h>
#include <initguid.h>
#include<vdssys.h>
#include <winioctl.h>
#include <virtdisk.h>


#pragma comment(lib,"uuid.lib")  

#pragma comment(lib, "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.14393.0\\um\\x64\\Virtdisk.Lib")

int main()
{
	WCHAR*							VhdxFilePath =L"f:\\123.vhdx";
	
	VIRTUAL_STORAGE_TYPE			VirtualStorageType;
	VIRTUAL_DISK_ACCESS_MASK        VirtualDiskAccessMask= VIRTUAL_DISK_ACCESS_ALL;

	OPEN_VIRTUAL_DISK_FLAG			ovdf = OPEN_VIRTUAL_DISK_FLAG_NONE;
	OPEN_VIRTUAL_DISK_PARAMETERS    ovdp;

	BOOL							bRet;
	HANDLE							hToken;
	
	ATTACH_VIRTUAL_DISK_FLAG		avdf = ATTACH_VIRTUAL_DISK_FLAG_PERMANENT_LIFETIME | ATTACH_VIRTUAL_DISK_FLAG_NO_DRIVE_LETTER;
	ATTACH_VIRTUAL_DISK_PARAMETERS	avdp;        

	VOLUME_DISK_EXTENTS				DiskExtents;
	DWORD							RetBytes;

	DWORD							RetCode = 0;
	HANDLE							hVirtualDisk;
	HANDLE							hFirstVolume = INVALID_HANDLE_VALUE;
	HANDLE							hVolumeCreateFile = INVALID_HANDLE_VALUE;

	CHAR                            PHYPath[128];
	WCHAR							VirtualDiskPhysicalPath[1024];
	ULONG							PathSize = 1024 * 2;
	DWORD							DiskNumber;

	CHAR							CurrentVolumeName[128];

	VirtualStorageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_VHDX;
	VirtualStorageType.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT;

	ovdp.Version = OPEN_VIRTUAL_DISK_VERSION_1;
	ovdp.Version1.RWDepth = OPEN_VIRTUAL_DISK_RW_DEPTH_DEFAULT;

	RetCode = OpenVirtualDisk(&VirtualStorageType, VhdxFilePath, VirtualDiskAccessMask, ovdf, &ovdp, &hVirtualDisk);
	if (RetCode != ERROR_SUCCESS)
	{
		printf("\nOpenVirtualDisk fail!Error code:%d",GetLastError());
	}
	else
	{
		printf("\nOpenVirtualDisk OK!");

		avdp.Version = ATTACH_VIRTUAL_DISK_VERSION_1;

		bRet = OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);
		if (bRet) 
		{
			TOKEN_PRIVILEGES	NewPriv; 

			NewPriv.PrivilegeCount = 1;//tokenPri.Privileges数组的大小

			NewPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;//开启权限Luid所标识的权限

			bRet = LookupPrivilegeValue(NULL, SE_MANAGE_VOLUME_NAME, &NewPriv.Privileges[0].Luid);//获取权限的标识Luid

			if (bRet) 
			{
				bRet = AdjustTokenPrivileges(hToken, FALSE, &NewPriv, sizeof(NewPriv), NULL, NULL);//修改权限}CloseHandle(tokenHandle);
				if (bRet != 0)
				{
					printf("\nAdjustTokenPrivileges OK!");
				}
				else
				{
					printf("\nAdjustTokenPrivileges fails!Error code %d",GetLastError());
				}
			}

		}

		RetCode = AttachVirtualDisk(hVirtualDisk, NULL, avdf, NULL, &avdp, NULL);
		if (RetCode == ERROR_SUCCESS)
		{
			printf("\nAttachVirtualDisk OK!");//系统多出一个新卷!NO_LETTER了!
			
			//尝试得到物理路径
			RetCode = GetVirtualDiskPhysicalPath(hVirtualDisk, &PathSize, VirtualDiskPhysicalPath);
			if (RetCode == ERROR_SUCCESS)
			{
				BOOL  FirstLetter = TRUE;
				printf("\nGetVirtualDiskPhysicalPath OK:%S", VirtualDiskPhysicalPath);

				{
					sprintf(PHYPath, "%ws", VirtualDiskPhysicalPath);

					DiskNumber = PHYPath[17] - 0x30;
				}

				hFirstVolume = FindFirstVolumeA(CurrentVolumeName, 128);//CreateFile 不要\,SetVolumeMountPoint需要
				do
				{
					CurrentVolumeName[strlen(CurrentVolumeName) - 1] = 0;
					hVolumeCreateFile = CreateFileA(CurrentVolumeName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
					if (hVolumeCreateFile == INVALID_HANDLE_VALUE)
					{
						printf("\nCreateFileA fails.Error code:%d", GetLastError());

						DebugBreak();
					}
					DeviceIoControl(hVolumeCreateFile, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, &DiskExtents, sizeof(DiskExtents), &RetBytes, NULL);

					CloseHandle(hVolumeCreateFile);

					// 如果卷跨越多个磁盘,就会找到多个Extents,VHDX不会!用此卷的DiskExtents的DiskNumber与自己的VDisk的号比较
					if (DiskExtents.Extents[0].DiskNumber == DiskNumber)//此卷位于我的磁盘
					{
						if (FirstLetter)
						{
							FirstLetter = FALSE;
						}
						else
						{
							CurrentVolumeName[strlen(CurrentVolumeName)] = '\\';
							bRet = SetVolumeMountPointA("V:\\", CurrentVolumeName);
							if (bRet)
							{
								printf("\nSetVolumeMountPointA OK!");
							}
							else
							{
								printf("\nSetVolumeMountPointA fails!Error Code:%d", GetLastError());
							}
						}
					}
				}while (FindNextVolumeA(hFirstVolume, CurrentVolumeName, 128));

				FindVolumeClose(hFirstVolume);
			}
			else
			{
				printf("\nGetVirtualDiskPhysicalPath fails...");
			}
		}
		else
		{
			printf("\nAttachVirtualDisk fails!Error code %d",GetLastError());
		}

		Sleep(5000);

		DetachVirtualDisk(hVirtualDisk, DETACH_VIRTUAL_DISK_FLAG_NONE, 0);
	
		CloseHandle(hVirtualDisk);
	}

	getchar();

	return 0;
}
