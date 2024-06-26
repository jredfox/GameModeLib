#Set DP0 ##
dir=$(dirname "$0")
rc="$dir/Resources/Mac"

echo "Installing GameModeLib Full Edition"

#Function Keys F1-F12 For the Current User Non Admin #
defaults write com.apple.keyboard.fnState -bool TRUE
bash "$rc/EnableFn.sh"

#Start Power Plan ##
echo "Setting Power Preferences"
#Disable Low Power Mode ##
sudo pmset -a lowpowermode 0
#Enable PowerNapping while Plugged In ##
sudo pmset -c powernap 1
sudo pmset -b powernap 0
#Change When Display Turns off #
sudo pmset -a displaysleep 30
sudo pmset -b displaysleep 15
#Always Use Dedicated Graphics ##
sudo pmset -a gpuswitch 1
#Set Battery Mode to Use Automatic ##
sudo pmset -b gpuswitch 2

#Changes Multi Media Keys to function Keys F1-F10 and F11-F12 When Shortcuts are Disabled #
defaults write -g com.apple.keyboard.fnState -bool TRUE

# Enables F11-F12 For New Users #
sudo ls -1 -d /Library/User\ Template/*/Library/Preferences/ | while read -r prefdir; do
	pl="${prefdir}com.apple.symbolichotkeys.plist"
	echo "Fn F11-F12 For New Users ${pl}"
	# Gen Uninstall PLIST Data #
	sudo cp -n "$pl" "${pl}.bak"
	# Fn F11 Show Desktop for New Users #
	sudo defaults write "$pl" AppleSymbolicHotKeys -dict-add 36 "<dict><key>enabled</key><false/></dict>" & sudo defaults write "$pl" AppleSymbolicHotKeys -dict-add 37 "<dict><key>enabled</key><false/></dict>"
	# Fn F12 Show DashBoard for New Users #
	sudo defaults write "$pl" AppleSymbolicHotKeys -dict-add 62 "<dict><key>enabled</key><false/></dict>" & sudo defaults write "$pl" AppleSymbolicHotKeys -dict-add 63 "<dict><key>enabled</key><false/></dict>"
done

# Enables F11-F12 For All Users #
sudo ls -1 "/Users" | while read -r uname; do
    if [[ "$uname" != "Shared" && "$uname" != "Deleted Users" &&  "$uname" != *"."* ]]; then
        echo "Fn F11-F12 for User $uname"
	sudo -u "$uname" defaults write "com.apple.symbolichotkeys.plist" AppleSymbolicHotKeys -dict-add 36 "<dict><key>enabled</key><false/></dict>"
	sudo -u "$uname" defaults write "com.apple.symbolichotkeys.plist" AppleSymbolicHotKeys -dict-add 37 "<dict><key>enabled</key><false/></dict>"
	
	sudo -u "$uname" defaults write "com.apple.symbolichotkeys.plist" AppleSymbolicHotKeys -dict-add 62 "<dict><key>enabled</key><false/></dict>"
	sudo -u "$uname" defaults write "com.apple.symbolichotkeys.plist" AppleSymbolicHotKeys -dict-add 63 "<dict><key>enabled</key><false/></dict>"
    fi
done
