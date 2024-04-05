#Enables F11 Show Desktop
defaults write "com.apple.symbolichotkeys.plist" AppleSymbolicHotKeys -dict-add 36 "<dict><key>enabled</key><false/></dict>" & defaults write "com.apple.symbolichotkeys.plist" AppleSymbolicHotKeys -dict-add 37 "<dict><key>enabled</key><false/></dict>"
#Enables F12 Show Dashboard
defaults write "com.apple.symbolichotkeys.plist" AppleSymbolicHotKeys -dict-add 62 "<dict><key>enabled</key><false/></dict>" & defaults write "com.apple.symbolichotkeys.plist" AppleSymbolicHotKeys -dict-add 63 "<dict><key>enabled</key><false/></dict>"
