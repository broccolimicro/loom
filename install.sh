#!/bin/sh

TAG=$(curl -Ls "https://api.github.com/repos/broccolimicro/loom/releases?per_page=1" | grep "\"name\": *\"v" | sed 's/.*: *"\(v[^"]*\)".*/\1/g')
OS=$(uname)

if [ "$OS" = "Linux" ]; then
	curl -L https://github.com/broccolimicro/loom/releases/download/$TAG/lm-linux.deb -o lm-linux.deb
	sudo dpkg -i lm-linux.deb
elif [ "$OS" = "Darwin" ]; then
	echo "Downloading Loom"
	curl -Ls https://github.com/broccolimicro/loom/releases/download/$TAG/lm-macos -o lm
	chmod 0755 lm
	echo "Copying Loom to /usr/local/bin/lm"
	sudo mv lm /usr/local/bin/lm
else
	curl -L https://github.com/broccolimicro/loom/releases/download/$TAG/lm-windows.zip -o lm-windows.zip
	unzip lm-windows.zip
fi

