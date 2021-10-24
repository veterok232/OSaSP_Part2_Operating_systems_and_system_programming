#include "RegistryModifyier.h"

const char FAIL_MESSAGE[] = "Error!\0";
const char SUCCESS_MESSAGE[] = "Success!\0";

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

	printf("Search result in %s\\%s\\:\n", lpsArguments[0], lpsArguments[1]);
	for (DWORD dwIndex = 0; dwIndex < dwFoundKeysCount; dwIndex++)
	{
		wprintf(L"%d. %s\n", dwIndex, lpsFoundKeys[dwIndex]);
	}

	return SUCCESS_MESSAGE;
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
		printf("Initialization key flags failure!\n");
	}
	else
	{
		if (!ParseRegExeOutput(lpsRegExeOutput, kfFlags, dwFlagsCount))
		{
			return FAIL_MESSAGE;
		}
	}

	printf("Key flags:\n");
	for (DWORD dwIndex = 0; dwIndex < dwFlagsCount; dwIndex++)
	{
		printf("%d. Flag name: %s  Flag value: %s\n", dwIndex, kfFlags[dwIndex].lpsFlagName, kfFlags[dwIndex].lpsFlagValue);
	}

	return SUCCESS_MESSAGE;
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
		return SUCCESS_MESSAGE;
	}
	else
	{
		return FAIL_MESSAGE;
	}
}

//Commands handler
LPCSTR CommandController(char** argv, int argc)
{
	if (argc < 2)
	{
		return FAIL_MESSAGE;
	}

	if (strcmp(argv[1], "ADD") == 0)
	{
		return AddDataCommand(argv + 2, argc - 2);
	}
	if (strcmp(argv[1], "CREATE_KEY") == 0)
	{
		return CreateKeyCommand(argv + 2, argc - 2);
	}
	if (strcmp(argv[1], "FLAGS") == 0)
	{
		return GetFlagsCommand(argv + 2, argc - 2);
	}
	if (strcmp(argv[1], "SEARCH") == 0)
	{
		return SearchCommand(argv + 2, argc - 2);
	}
	if (strcmp(argv[1], "NOTIFY") == 0)
	{
		return NotifyCommand(argv + 2, argc - 2);
	}

	return NULL;
}


int main(int argc, char** argv)
{
	LPCSTR lpsCmdResult = CommandController(argv, argc);

	if (lpsCmdResult == NULL)
	{
		printf("Unknown command!\n");
	}
	else
	{
		printf("%s\n", lpsCmdResult);
	}

	getchar();
	return 0;

	/*HKEY key1;
	HKEY key2;

	if (!OpenRegKey(HKEY_USERS, L".DEFAULT\\Control Panel\\Colors", KEY_READ, &key1))
	{
		printf("Open key failure!\n");
	}

	if (!CreateRegKey(HKEY_USERS, L".DEFAULT\\Software\\Test_key"))
	{
		printf("Create key failure!\n");
	}

	if (!CreateRegKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Test_key"))
	{
		printf("Create key failure!\n");
	}

	if (!SetRegKey(HKEY_USERS, L".DEFAULT\\Test_key", L"Test1", REG_SZ, (LPCVOID)L"Hello world!\0", sizeof(L"Hello world!\0")))
	{
		printf("Set key value failure!\n");
	}

	int binaryValue2 = 12345;
	int dwordValue2 = 12345;

	if (!SetRegKey(HKEY_USERS, L".DEFAULT\\Test_key", L"Test2", REG_BINARY, (LPCVOID)&binaryValue2, sizeof(&binaryValue2)))
	{
		printf("Set key value failure!\n");
	}

	if (!SetRegKey(HKEY_USERS, L".DEFAULT\\Test_key", L"Test3", REG_DWORD, (LPCVOID)&dwordValue2, sizeof(&dwordValue2)))
	{
		printf("Set key value failure!\n");
	}

	LPWSTR keyValue = (LPWSTR)malloc(4096);
	DWORD keyValueSize = 4096;
	DWORD dwParamType;

	if (!GetRegKey(HKEY_USERS, L".DEFAULT\\Test_key", L"Test1", &dwParamType, (BYTE*)keyValue, &keyValueSize))
	{
		printf("Get key value failure!\n");
	}
	wprintf(L"Test_key.Test1 : %s\n", keyValue);

	if (!GetRegKey(HKEY_USERS, L".DEFAULT\\Control Panel\\Colors", L"ActiveBorder", &dwParamType, (BYTE*)keyValue, &keyValueSize))
	{
		printf("Get key value failure!\n");
	}
	wprintf(L"Colors.ActiveBorder : %s\n", keyValue);

	HKEY key3;

	if (!OpenRegKey(HKEY_USERS, L".DEFAULT", KEY_READ, &key3))
	{
		printf("Open key value failure!\n");
	}

	DWORD dwFoundKeysCount = 0;
	LPWSTR* lpsFoundKeys = SearchKey(key3, L"Test_key", &dwFoundKeysCount);

	wprintf(L"Search result:\n");
	for (DWORD dwIndex = 0; dwIndex < dwFoundKeysCount; dwIndex++)
	{
		wprintf(L"%d. %s\n", dwIndex, lpsFoundKeys[dwIndex]);
	}

	WCHAR lpsCommand[] = L"REG FLAGS HKLM\\SOFTWARE\\Test_key QUERY";
	LPSTR lpsRegExeOutput = ExecuteRegExe(lpsCommand);
	printf("%s\n", lpsRegExeOutput);

	KEYFLAG* kfFlags = NULL;
	DWORD dwFlagsCount;

	kfFlags = GetInitializedFlags(&dwFlagsCount);

	if (kfFlags == NULL)
	{
		printf("Initialization key flags failure!\n");
	}
	else
	{
		ParseRegExeOutput(lpsRegExeOutput, kfFlags, dwFlagsCount);
	}

	for (DWORD dwIndex = 0; dwIndex < dwFlagsCount; dwIndex++)
	{
		printf("%d. Flag name: %s  Flag value: %s\n", dwIndex, kfFlags[dwIndex].lpsFlagName, kfFlags[dwIndex].lpsFlagValue);
	}

	CloseRegKey(key1);
	getchar();
	return 0;*/
}