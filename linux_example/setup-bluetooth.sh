#!/bin/bash

# Ensure the script is run as root
if [ "$EUID" -ne 0 ]; then
  echo "Please run as root or using sudo."
  exit 1
fi

echo "Checking Bluetooth service status..."
# Make sure the bluetooth service is enabled and running
systemctl enable --now bluetooth

echo "Unblocking Bluetooth via rfkill..."
rfkill unblock bluetooth

echo "Configuring Bluetooth visibility and pairing..."
# Use bluetoothctl to configure the controller
bluetoothctl <<EOF
power on
discoverable on
pairable on
discoverable-timeout 0
exit
EOF

sudo sdptool add SP

echo "------------------------------------------------"
echo "Bluetooth is now ON, DISCOVERABLE, and PAIRABLE."
echo "Discoverable timeout is disabled (will stay visible)."
echo "Serial Port is registered"
echo "------------------------------------------------"
