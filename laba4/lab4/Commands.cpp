#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include "framework.h"
#include "lab4.h"


const char FAIL_MESSAGE[] = "Error.\0";
const char SUCCESS_MESSAGE[] = "Executed.\0";

//Convert char* to wchar_t*
const wchar_t* GetWC(const char* c)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	size_t outSize;
	mbstowcs_s(&outSize, wc, cSize, c, cSize - 1);
	return wc;
}

//Get HKEY from its string representation
HKEY GetHkeyRoot(LPSTR lpsKey)
{
	if (strcmp(lpsKey, "HKEY_CLASSES_ROOT") == 0)
	{
		return HKEY_CLASSES_ROOT;
	}
	if (strcmp(lpsKey, "HKEY_CURRENT_USER") == 0)
	{
		return HKEY_CURRENT_USER;
	}
	if (strcmp(lpsKey, "HKEY_LOCAL_MACHINE") == 0)
	{
		return HKEY_LOCAL_MACHINE;
	}
	if (strcmp(lpsKey, "HKEY_USERS") == 0)
	{
		return HKEY_USERS;
	}
	if (strcmp(lpsKey, "HKEY_CURRENT_CONFIG") == 0)
	{
		return HKEY_CURRENT_CONFIG;
	}
	return NULL;
}

//Get registry type from its string representation
DWORD GetParamType(LPSTR lpsType)
{
	if (strcmp(lpsType, "REG_SZ") == 0)
	{
		return REG_SZ;
	}
	if (strcmp(lpsType, "REG_BINARY") == 0)
	{
		return REG_BINARY;
	}
	if (strcmp(lpsType, "REG_DWORD") == 0)
	{
		return REG_DWORD;
	}
	if (strcmp(lpsType, "REG_LINK") == 0)
	{
		return REG_LINK;
	}
	return NULL;
}

//Convert value of registry type to LPVOID
LPCVOID ConvertValueToLPVOID(DWORD dwParamType, LPSTR lpsValue, DWORD* dwValueSize)
{
	switch (dwParamType)
	{
	case REG_SZ:
	{
		LPVOID lpBuffer = (LPVOID)GetWC(lpsValue);
		*dwValueSize = lstrlen(GetWC(lpsValue)) * sizeof(WCHAR);
		return lpBuffer;
	}
	case REG_DWORD:
	{
		DWORD* lpdwValue = (DWORD*)calloc(1, sizeof(DWORD));
		*lpdwValue = atoi(lpsValue);
		*dwValueSize = sizeof(*lpdwValue);
		return (LPVOID)lpdwValue;
	}
	case REG_BINARY:
	{
		LPVOID lpBuffer = (LPVOID)lpsValue;
		*dwValueSize = strlen(lpsValue) * sizeof(char);
		return lpBuffer;
	}
	case REG_LINK:
	{
		LPVOID lpBuffer = (LPVOID)GetWC(lpsValue);
		*dwValueSize = lstrlen(GetWC(lpsValue)) * sizeof(WCHAR);
		return lpBuffer;
	}
	}

	return NULL;
}

//Add data to key command
LPCSTR AddDataCommand(LPSTR* lpsArguments, DWORD dwArgumentsCount)
{
	if (dwArgumentsCount < 5)
	{
		return FAIL_MESSAGE;
	}

	DWORD dwArgumentSize;
	LPCVOID lpBuffer = ConvertValueToLPVOID(GetParamType(lpsArguments[3]), lpsArguments[4], &dwArgumentSize);

	if (lpBuffer == NULL)
	{
		return FAIL_MESSAGE;
	}

	if (SetRegKey(GetHkeyRoot(lpsArguments[0]),
		GetWC(lpsArguments[1]),
		GetWC(lpsArguments[2]),
		GetParamType(lpsArguments[3]),
		lpBuffer,
		dwArgumentSize))
	{
		return SUCCESS_MESSAGE;
	}
	else
	{
		return FAIL_MESSAGE;
	}
}

//Create a new key command
LPCSTR CreateKeyCommand(LPSTR* lpsArguments, DWORD dwArgumentsCount)
{
	if (dwArgumentsCount < 2)
	{
		return FAIL_MESSAGE;
	}

	HKEY hKeyRoot = GetHkeyRoot(lpsArguments[0]);
	LPCWSTR lpsSubkeyPath = GetWC(lpsArguments[1]);

	if ((hKeyRoot == NULL) || (lpsSubkeyPath == NULL))
	{
		return FAIL_MESSAGE;
	}

	if (CreateRegKey(hKeyRoot, lpsSubkeyPath))
	{
		return SUCCESS_MESSAGE;
	}
	else
	{
		return FAIL_MESSAGE;
	}
}

//Search key command
LPCSTR SearchCommand(LPSTR* lpsArguments, DWORD dwArgumentsCount)
{
	if (dwArgumentsCount < 3)
	{
		return FAIL_MESSAGE;
	}

	HKEY hKey;

	if (!OpenRegKey(GetHkeyRoot(lpsArguments[0]), GetWC(lpsArguments[1]), KEY_READ, &hKey))
	{
		return FAIL_MESSAGE;
	}

	DWORD dwFoundKeysCount = 0;
	LPWSTR* lpsFoundKeys = SearchKey(hKey, GetWC(lpsArguments[2]), &dwFoundKeysCount);

	if (lpsFoundKeys == NULL)
	{
		return "No keys found!\n";
	}
	wchar_t buf[20000] = L"", buf2[2000];
	swprintf(buf, L"Search result in %s\\%s\\:\n", GetWC(lpsArguments[0]), GetWC(lpsArguments[1]));
	for (DWORD dwIndex = 0; dwIndex < dwFoundKeysCount; dwIndex++)
	{
		swprintf(buf2,L"%d. %s\n", (int)dwIndex, lpsFoundKeys[dwIndex]);
		wcscat(buf,buf2);
	}
	char buf3[20000 ];
	for (int i = 0; i < wcslen(buf);i++) 
	{
		buf3[i] =static_cast<char>(buf[i]);

	}
	buf3[wcslen(buf) - 1] = 0;
	return buf3;
}

//Get key flags command
LPCSTR GetFlagsCommand(LPSTR* lpsArguments, DWORD dwArgumentsCount)
{
	if (dwArgumentsCount < 2)
	{
		return FAIL_MESSAGE;
	}

	WCHAR* lpsCommand = CreateFlagsQuery(GetWC(lpsArguments[0]), GetWC(lpsArguments[1]));//L"REG FLAGS HKLM\\SOFTWARE\\Test_key QUERY";

	if (lpsCommand == NULL)
	{
		return FAIL_MESSAGE;
	}

	LPSTR lpsRegExeOutput = ExecuteRegExe(lpsCommand);

	if (lpsRegExeOutput == NULL)
	{
		return FAIL_MESSAGE;
	}

	KEYFLAG* kfFlags = NULL;
	DWORD dwFlagsCount;

	kfFlags = GetInitializedFlags(&dwFlagsCount);

	if (kfFlags == NULL)
	{
		return "Initialization key flags failure!\n";
	}
	else
	{
		if (!ParseRegExeOutput(lpsRegExeOutput, kfFlags, dwFlagsCount))
		{
			return "Cannot read flags";
		}
	}
	char buf[1024] = "";
	char buf2[128];
	//sprintf(buf,"Key flags:\n");
	for (DWORD dwIndex = 0; dwIndex < dwFlagsCount; dwIndex++)
	{
		sprintf(buf2,"%d.%s -- %s\n", dwIndex, kfFlags[dwIndex].lpsFlagName, kfFlags[dwIndex].lpsFlagValue);
		strcat(buf, buf2);
	}
	
	return buf;
}

//Notify command
LPCSTR NotifyCommand(LPSTR* lpsArguments, DWORD dwArgumentsCount)
{
	if (dwArgumentsCount < 2)
	{
		return FAIL_MESSAGE;
	}

	HKEY hKeyRoot = GetHkeyRoot(lpsArguments[0]);

	if (hKeyRoot == NULL)
	{
		return FAIL_MESSAGE;
	}

	LPCWSTR lpsSubkeyPath = GetWC(lpsArguments[1]);

	if (NotifyChange(hKeyRoot, lpsSubkeyPath, TRUE))
	{
		return "Changed";
	}
	else
	{
		return FAIL_MESSAGE;
	}
}

//Commands handler
LPCSTR CommandController(char** arguments, int argc)
{
	if (argc < 2)
	{
		return FAIL_MESSAGE;
	}

	if (strcmp(arguments[0], "ADD") == 0)
	{
		return AddDataCommand(arguments + 1, argc - 1);
	}
	if (strcmp(arguments[0], "CREATE_KEY") == 0)
	{
		return CreateKeyCommand(arguments + 1, argc - 1);
	}
	if (strcmp(arguments[0], "FLAGS") == 0)
	{
		return GetFlagsCommand(arguments + 1, argc - 1);
	}
	if (strcmp(arguments[0], "SEARCH") == 0)
	{
		return SearchCommand(arguments + 1, argc - 1);
	}
	if (strcmp(arguments[0], "NOTIFY") == 0)
	{
		return NotifyCommand(arguments + 1, argc - 1);
	}

	return NULL;
}
