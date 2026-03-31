import platform
import subprocess
import sys

os_name = platform.system()

if os_name == "Linux":
    print("Linux detected. Native USB passthrough will be used.")
    sys.exit(0)

elif os_name == "Darwin": # macOS
    print("macOS detected. Starting socat TCP bridge...")
    # Checks if socat is installed
    if subprocess.call("command -v socat", shell=True) != 0:
        print("Please install socat: brew install socat")
        sys.exit(1)
    
    # Kills any existing socat processes, then starts a new one forwarding the USB
    # Note: You may need to tweak the grep string to match your specific RoveSoPnP board name
    subprocess.Popen("pkill socat; socat file:$(ls /dev/tty.usbserial* | head -n 1),b115200,raw,echo=0 tcp-listen:4000,reuseaddr", shell=True)
    print("macOS USB bridge active on port 4000.")

elif os_name == "Windows":
    print("Windows detected. Attempting to bind USB to WSL...")
    # This attempts to attach the first attached USB Serial device. 
    # Note: Windows users will get a UAC prompt here if not already admin.
    subprocess.call('powershell -Command "usbipd attach --wsl --auto-attach"', shell=True)