#!/bin/bash

# If the container is running on a Mac host, it needs to connect to the host's socat port
# We can check if the host port 4000 is reachable
if nc -z host.docker.internal 4000 2>/dev/null; then
    echo "Connecting virtual serial port to macOS host..."
    sudo apt-get update && sudo apt-get install -y socat
    sudo socat pty,link=/dev/ttyV0,raw,echo=0 tcp:host.docker.internal:4000 &
else
    echo "Linux/Windows native USB will be used."
fi