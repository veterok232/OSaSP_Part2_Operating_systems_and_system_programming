#define _CRT_SECURE_NO_WARNINGS
#include "framework.h"
#include "lab4.h"

#define ADD_COMMAND 0
#define CREATE_KEY_COMMAND 1
#define FLAGS_COMMAND 2
#define SEARCH_COMMAND 3
#define NOTIFY_COMMAND 4

const wchar_t* GetWC(const char* c);
void ChangeEdits(bool root, bool path, bool paramName, bool paramType, bool data, bool nameOfKey);
void ShowMessage(const wchar_t mess[50], HWND hWnd);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HWND rootEdit;
HWND pathToSubkeyEdit;
HWND nameOfKeyEdit;
HWND paramNameEdit;
HWND paramTypeEdit;
HWND dataEdit;
HWND Combobox;
HWND hlist;
HWND executeButton;
wchar_t windowMessage[50] = L"";

bool rootNeeded = true, pathToSubkeyNeeded = true, paramNameNeeded = true, paramTypeNeeded = true, dataNeeded = true, nameOfKeyNeeded = false;
int condition = 0;

/*ADD[ROOT][Path_to_subkey][Param_name][Param_type][Data]
    CREATE_KEY[ROOT][Path_to_subkey]
    FLAGS[ROOT][Path_to_subkey]
    SEARCH[ROOT][Path_to_subkey][Name_of_key]
    NOTIFY[ROOT][Path_to_subkey]*/

int WINAPI wWinMain  (HINSTANCE hInstance, HINSTANCE, LPTSTR, int nCmdShow)
{
    WNDCLASS windowClass = { 0 };
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = L"HELLO_WORLD";
    windowClass.hbrBackground = (HBRUSH)GetStockObject(COLOR_WINDOW + 1);
    RegisterClass(&windowClass);
    HWND hwnd = CreateWindow(
        windowClass.lpszClassName,
        L"4lab",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        100, 50, 600, 480,
        nullptr, nullptr,
        hInstance,
        nullptr);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return 0;
}

//int main(int argc, char** argv)
//{
//    LPCSTR lpsCmdResult = CommandController(argv, argc);
//
//    if (lpsCmdResult == NULL)
//    {
//        printf("Unknown command!\n");
//    }
//    else
//    {
//        printf("%s\n", lpsCmdResult);
//    }
//
//    getchar();
//    return 0;
//}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

    case WM_CREATE:
    {
        rootEdit = CreateWindowEx(NULL, L"Edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
            10, 20, 250, 30, hWnd, (HMENU)2, (HINSTANCE)GetWindowLongA(hWnd, -6), NULL);
        pathToSubkeyEdit = CreateWindowEx(NULL, L"Edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
            10, 80, 250, 30, hWnd, (HMENU)3, (HINSTANCE)GetWindowLongA(hWnd, -6), NULL);
        paramNameEdit = CreateWindowEx(NULL, L"Edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
            10, 140, 250, 30, hWnd, (HMENU)5, (HINSTANCE)GetWindowLongA(hWnd, -6), NULL);
        paramTypeEdit = CreateWindowEx(NULL, L"combobox", NULL, WS_CHILD | WS_VISIBLE | CBS_HASSTRINGS | CBS_DROPDOWNLIST,
            10, 200, 250, 210, hWnd, (HMENU)6, (HINSTANCE)GetWindowLongA(hWnd, -6), NULL);
        dataEdit = CreateWindowEx(NULL, L"Edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
            10, 260, 250, 30, hWnd, (HMENU)7, (HINSTANCE)GetWindowLongA(hWnd, -6), NULL);
        nameOfKeyEdit = CreateWindowEx(NULL, L"Edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
            10, 320, 250, 30, hWnd, (HMENU)8, (HINSTANCE)GetWindowLongA(hWnd, -6), NULL);
        ShowWindow(nameOfKeyEdit, SW_HIDE);
        
        
        hlist = CreateWindow(L"listbox", NULL, WS_CHILD | WS_VISIBLE| WS_VSCROLL |WS_BORDER | LBS_NOTIFY,
             280, 30, 300, 270, hWnd, (HMENU)8, (HINSTANCE)GetWindowLongA(hWnd, -6), NULL);
        
        executeButton = CreateWindow(L"button", L"Execute", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 360, 120, 30, hWnd, (HMENU)4, (HINSTANCE)GetWindowLongA(hWnd, -6), NULL);

        Combobox = CreateWindow(L"combobox", L"command", WS_CHILD | WS_VISIBLE | CBS_HASSTRINGS | CBS_DROPDOWNLIST
            | WS_OVERLAPPED | WS_VSCROLL, 360, 0, 120, 120, hWnd, (HMENU)11, (HINSTANCE)GetWindowLongA(hWnd, -6), NULL);

        SendMessage(hlist, WM_SETREDRAW, TRUE, 0L);
        SendMessage(Combobox, CB_ADDSTRING, 0, (LPARAM)L"Add parameter ");
        SendMessage(Combobox, CB_ADDSTRING, 0, (LPARAM)L"Create section");
        SendMessage(Combobox, CB_ADDSTRING, 0, (LPARAM)L"View Flags");
        SendMessage(Combobox, CB_ADDSTRING, 0, (LPARAM)L"Search key");
        SendMessage(Combobox, CB_ADDSTRING, 0, (LPARAM)L"Notify changes");

        SendMessage(Combobox, WM_SETREDRAW, TRUE, 0L);
        SendMessage(Combobox, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

        SendMessage(paramTypeEdit, CB_ADDSTRING, 0, (LPARAM)L"REG_SZ");
        SendMessage(paramTypeEdit, CB_ADDSTRING, 0, (LPARAM)L"REG_DWORD");
        SendMessage(paramTypeEdit, CB_ADDSTRING, 0, (LPARAM)L"REG_BINARY");
        SendMessage(paramTypeEdit, CB_ADDSTRING, 0, (LPARAM)L"REG_LINK");
        SendMessage(paramTypeEdit, WM_SETREDRAW, TRUE, 0L);
        SendMessage(paramTypeEdit, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

        
        /*ADD[ROOT][Path_to_subkey][Param_name][Param_type][Data]
            CREATE_KEY[ROOT][Path_to_subkey]
            FLAGS[ROOT][Path_to_subkey]
            SEARCH[ROOT][Path_to_subkey][Name_of_key]
            NOTIFY[ROOT][Path_to_subkey]*/
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC winDC = BeginPaint(hWnd, &ps);
        if (rootNeeded) 
        {
            TextOut(winDC, 10, 3, L"ROOT", 4);
  
        }
        if (pathToSubkeyNeeded) 
        {
            TextOut(winDC, 10, 60, L"Path to subkey", 15);
        }
        if (paramNameNeeded) 
        {
            TextOut(winDC, 10, 120, L"Parameter name", 15);
        }
        if (paramTypeNeeded) 
        {
            TextOut(winDC, 10, 180, L"Parameter type", 15);
        }
        if (dataNeeded) 
        {
            TextOut(winDC, 10, 240, L"Data", 4);
        }
        if (nameOfKeyNeeded )
        {
            TextOut(winDC, 10, 300, L"Name of key", 12);
        }

        
        TextOut(winDC, 280, 3, L"Command", 8);
        TextOut(winDC, 10, 400, windowMessage, lstrlenW(windowMessage));
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_COMMAND:
    {
        if (HIWORD(wParam) == CBN_SELCHANGE)
        {
            int itemInd = SendMessage(Combobox, CB_GETCURSEL, 0, 0);
            switch (itemInd)
            {
            case 0:
               // ShowMessage(L"Add command ", hWnd);
                condition = ADD_COMMAND;
                ChangeEdits(true, true, true, true, true,false);
                break;
            case 1:
                condition = CREATE_KEY_COMMAND;
                ChangeEdits(true, true, false, false, false, false);
                break;
            case 2:
                condition = FLAGS_COMMAND;
                ChangeEdits(true, true, false, false, false, false);
                break;
            case 3:
                condition = SEARCH_COMMAND;
                ChangeEdits(true, true, false, false, false, true);
                break;
            case 4:

                condition = NOTIFY_COMMAND;
                ChangeEdits(true, true, false, false, false, false);
                break;

            default:
                condition = 0;
                MessageBox(NULL, L"Error.DLL impory type changed to STATIC", L"ERROR", MB_OK);
                break;
            }
        }

        if (HIWORD(wParam) == BN_CLICKED)
        {

            switch (LOWORD(wParam))
            {

            case 4:
            {
               
//
//    if (lpsCmdResult == NULL)
//    {
//        printf("Unknown command!\n");
//    }
//    else
//    {
//        printf("%s\n", lpsCmdResult);
//    }
               
                char root[MAX_CLASS_NAME];
                GetWindowTextA((HWND)rootEdit, &root[0], 199);
                const char* replaced = root;
                char path[MAX_CLASS_NAME];
                GetWindowTextA((HWND)pathToSubkeyEdit, &path[0], 199);
                const char* pathtoSk = path;

                int result = -1;
                LPCSTR lpsCmdResult ="dwqwdq";
                switch (condition)
                {

                case ADD_COMMAND:
                {
                    char** argv;
                    int argc = 6;
                    argv = (char**)malloc(sizeof(char**)*argc);
                    for (int i = 0; i < argc; i++)
                    {
                        argv[i] = (char*)malloc(sizeof(char) * 200);
                    }

                    strcpy(argv[0], "ADD");
                    GetWindowTextA((HWND)rootEdit, argv[1], 199);
                    GetWindowTextA((HWND)pathToSubkeyEdit, argv[2], 199);
                    GetWindowTextA((HWND)paramNameEdit, argv[3], 199);
                    GetWindowTextA((HWND)paramTypeEdit, argv[4], 199);
                    GetWindowTextA((HWND)dataEdit, argv[5], 199);
                    
                    
                     lpsCmdResult = CommandController(argv, argc);

                    //SendMessage(hlist, LB_ADDSTRING, 0, (LPARAM)L"asddsa");
                    break;
                }
                case CREATE_KEY_COMMAND:
                {
                    char** argv;
                    int argc = 3;
                    argv = (char**)malloc(sizeof(char**) * argc);
                    for (int i = 0; i < argc; i++)
                    {
                        argv[i] = (char*)malloc(sizeof(char) * 200);
                    }
                    strcpy(argv[0], "CREATE_KEY");
                    GetWindowTextA((HWND)rootEdit, argv[1], 199); \
                    GetWindowTextA((HWND)pathToSubkeyEdit, argv[2], 199);
                    lpsCmdResult = CommandController(argv, argc);
                    break;
                }
                case FLAGS_COMMAND:
                {
                    char** argv;
                    int argc = 3;
                    argv = (char**)malloc(sizeof(char**) * argc);
                    for (int i = 0; i < argc; i++)
                    {
                        argv[i] = (char*)malloc(sizeof(char) * 200);
                    }
                    strcpy(argv[0], "FLAGS");
                    GetWindowTextA((HWND)rootEdit, argv[1], 199); \
                        GetWindowTextA((HWND)pathToSubkeyEdit, argv[2], 199);
                   lpsCmdResult = CommandController(argv, argc);
                
                    break;
                }
                case SEARCH_COMMAND:
                {
                    char** argv;
                    int argc = 4;
                    argv = (char**)malloc(sizeof(char**) * argc);
                    for (int i = 0; i < argc; i++)
                    {
                        argv[i] = (char*)malloc(sizeof(char) * 200);
                    }
                    strcpy(argv[0], "SEARCH");
                    GetWindowTextA((HWND)rootEdit, argv[1], 199); \
                        GetWindowTextA((HWND)pathToSubkeyEdit, argv[2], 199);
                    GetWindowTextA((HWND)nameOfKeyEdit, argv[3], 199);
                    lpsCmdResult = CommandController(argv, argc);
                    


                    break;
                }
                case NOTIFY_COMMAND:
                {
                    char** argv;
                    int argc = 3;
                    argv = (char**)malloc(sizeof(char**) * argc);
                    for (int i = 0; i < argc; i++)
                    {
                        argv[i] = (char*)malloc(sizeof(char) * 200);
                    }
                    strcpy(argv[0], "NOTIFY");
                    GetWindowTextA((HWND)rootEdit, argv[1], 199); \
                        GetWindowTextA((HWND)pathToSubkeyEdit, argv[2], 199);
                    lpsCmdResult = CommandController(argv, argc);
                    break;
                }

                default:
                    break;
                }

                wchar_t buf[1024];
                for (int i = 0; i < 1024; i++) 
                {
                   
                    buf[i] = lpsCmdResult[i];
                    if (lpsCmdResult[i] == 0)

                        break;
                }
                MessageBox(NULL, (LPCWSTR)buf, L"Result", MB_OK);
                if (wcslen(buf) > 36) 
                {
                    int t = 0;
                    wchar_t tmp[35];
                    for (int i = 0; i < wcslen(buf);i++) 
                    {
                        tmp[t] = buf[i];
                        t++;
                        if (t == 34) 
                        {
                            tmp[34] ='\0';
                            t = 0;
                            t = SendMessage(hlist, LB_ADDSTRING, 0, (LPARAM)tmp);
                            t = 0;
                        }
                    
                    
                    }
                    if (t != 0) 
                    {
                        tmp[t] = 0;
                        SendMessage(hlist, LB_ADDSTRING, 0, (LPARAM)(LPCWSTR)tmp);

                    }
                
                
                
                }
                else
                    SendMessage(hlist, LB_ADDSTRING, 0, (LPARAM)(LPCWSTR)buf);
                //MessageBox(NULL, buf, L"Result", MB_OK);
                break;
            }
            default:
                break;
            }
        }
        break;
    }
    break;

    case WM_DESTROY:
    {
        PostQuitMessage(0);
    }
    break;
    default: {
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    }
    return 0;
}

void ShowMessage(const wchar_t mess[50], HWND hWnd)
{
    lstrcpyW(windowMessage, mess);
    InvalidateRect(hWnd, NULL, TRUE);
}

void ChangeEdits(bool root, bool path, bool paramName, bool paramType, bool data, bool nameOfKey) 
{
    rootNeeded = root;
    pathToSubkeyNeeded = path;
    paramNameNeeded = paramName;
    paramTypeNeeded = paramType;
    dataNeeded = data;
    nameOfKeyNeeded = nameOfKey;
    
    if (dataNeeded)
    {
        ShowWindow(dataEdit, SW_SHOW);
    }
    else
    {
        ShowWindow(dataEdit, SW_HIDE);
    }


    if (rootNeeded) 
    {
        ShowWindow(rootEdit, SW_SHOW);
    }
    else
        ShowWindow(rootEdit, SW_HIDE);

    if (pathToSubkeyNeeded)
    {
        ShowWindow(pathToSubkeyEdit, SW_SHOW);
    }
    else
        ShowWindow(pathToSubkeyEdit, SW_HIDE);

    if (paramNameNeeded)
    {
        ShowWindow(paramNameEdit, SW_SHOW);
    }
    else
        ShowWindow(paramNameEdit, SW_HIDE);

    if (paramTypeNeeded)
    {
        ShowWindow(paramTypeEdit, SW_SHOW);
    }
    else
        ShowWindow(paramTypeEdit, SW_HIDE);




    if (nameOfKeyNeeded)
    {
        ShowWindow(nameOfKeyEdit, SW_SHOW);
    }
    else
        ShowWindow(nameOfKeyEdit, SW_HIDE);

    InvalidateRect(NULL, NULL, TRUE);

}






