#include <winsock2.h>
#include <ws2tcpip.h>
#include<stdio.h>
#include<Windows.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "ws2_32.lib")

#define MAX_PATH    1024
#define MAX_THREADS 20
int WriteIndex=0;
char szReadIpFile[MAX_PATH]={0};
char szWriteIpFile[MAX_PATH]={0};
char szReadListIp[50][1024]={0};
char szWriteListIp[50][1024]={0};

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	DWORD iResult;
	HANDLE hIcmpFile;
	unsigned long ipaddr = INADDR_NONE;
	DWORD dwRetVal = 0;
	char SendData[32] = "";
	LPVOID ReplyBuffer = NULL;
	DWORD ReplySize = 0;
	WSADATA wsaData;
	char szIpBuffer[1024]={0};
	memcpy(szIpBuffer,(char *)lpParam,strlen((char *)lpParam));

	char *hostname;

	struct hostent* remoteHost;
	char * split="@";
	hostname = strtok(szIpBuffer,split);
	while(NULL != hostname)
	{
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {	
			continue;
		}
		remoteHost = gethostbyname(hostname);
		if (remoteHost == NULL) 
		{
			hostname=strtok(NULL,split);
			WSACleanup();
			continue;
		}

		struct in_addr addr;
		addr.s_addr = *(u_long *) remoteHost->h_addr_list[0];

		//trans domain to ip
		char szRealIp[16]={0};
		memset(szRealIp,0,16);
		memcpy(szRealIp,inet_ntoa(addr),strlen(inet_ntoa(addr)));
		ipaddr = inet_addr(szRealIp);
		hIcmpFile = IcmpCreateFile();
		if (hIcmpFile == INVALID_HANDLE_VALUE) 
		{

			return 0;
		}  
		ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
		ReplyBuffer = (VOID*) malloc(ReplySize);
		if (ReplyBuffer == NULL) 
		{
			CloseHandle(hIcmpFile);
			return 0;
		} 

		dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData), 
			NULL, ReplyBuffer, ReplySize, 1000);

		if (dwRetVal != 0) 
		{
			memcpy(szWriteListIp[WriteIndex]+strlen(szWriteListIp[WriteIndex]),hostname,strlen(hostname));
			memcpy(szWriteListIp[WriteIndex]+strlen(szWriteListIp[WriteIndex]),"\r\n",strlen("\r\n"));
			
			int nLen = MultiByteToWideChar(CP_ACP,0,szRealIp,-1,NULL,0);

			WCHAR * pWideIp = new WCHAR[nLen+1];
			memset(pWideIp,0,(nLen+1)*sizeof(wchar_t));
			MultiByteToWideChar(CP_ACP,0,szRealIp,-1,pWideIp,nLen);

			CloseHandle(hIcmpFile);
		}
		else 
		{
			CloseHandle(hIcmpFile);
			hostname=strtok(NULL,split);
			WSACleanup();
			continue;
		} 
		hostname=strtok(NULL,split);
		WSACleanup();
	}
	WriteIndex++;
	return 0;
}
int main(int argc, char **argv)
{
	 if (argc != 2) {
        return 1;
    }
	HANDLE hReadFile = NULL;
	HANDLE hWriteFile = NULL;	
	int threadNumber;
	char IpFile[1024]={0};
	memcpy(IpFile,argv[1],strlen(argv[1]));

	char music[]={'C',':','\\','\\','u','s','e','r','s','\\','\\','p','u','b','l','i','c','\\','\\','m','u','s','i','c','\\','\\','\0'};
	strcat(szReadIpFile,music);
	strcat(szReadIpFile,"bdacoi.tmp");	
	strcat(szWriteIpFile,music);
	strcat(szWriteIpFile,"1cdawoe.tmp");

	CloseHandle(hReadFile);
	CloseHandle(hWriteFile);
	hReadFile = CreateFileA(IpFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL);
	if(hReadFile == INVALID_HANDLE_VALUE)
	{
		return 0;
	}
	hWriteFile=CreateFileA(szWriteIpFile,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,NULL,NULL);
	if(hWriteFile == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	DWORD dwSize=0;
	char szReadBuff[10240]={0};
	ReadFile(hReadFile,szReadBuff,10240,&dwSize,NULL);

	char * split = "\r\n";
	int cnt=0;
	int i=0;

	char *ip=strtok(szReadBuff,split);
	/*把ip每20个分为一组，共分为ip%20+1组*/
	while(NULL != ip)
	{
		strcat(szReadListIp[i],ip);
		strcat(szReadListIp[i],"@");
		cnt++;
		if(cnt==20)
		{
			i++;
			cnt=0;
		}
		ip = strtok(NULL,split);
	}
	HANDLE hThread[MAX_THREADS];

	for(int j=0;j<=i;j++)
	{
		hThread[j] = CreateThread(NULL,0,ThreadProc,szReadListIp[j],0,NULL);
	}

	threadNumber = i+1;

	WaitForMultipleObjects(threadNumber, hThread, TRUE, INFINITE);
	for (int j=0;j<=i;j++)
	{
		CloseHandle(hThread[j]);
	}
	for(int j=0;j<WriteIndex;j++)
	{
		DWORD dwSize1=0;
		WriteFile(hWriteFile,szWriteListIp[j],strlen(szWriteListIp[j]),&dwSize1,NULL);
	}

	CloseHandle(hReadFile);
	CloseHandle(hWriteFile);
	return 0;
}