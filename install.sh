#!/bin/sh

TAG=$(curl -Ls https://api.github.com/repos/broccolimicro/loom/releases | jq -r 'first.name')
OS=$(uname)

if [ "$OS" = "Linux" ]; then
	curl -L https://github.com/broccolimicro/loom/releases/download/$TAG/lm-linux.deb -o lm-linux.deb
	sudo dpkg -i lm-linux.deb
elif [ "$OS" = "Darwin" ]; then
	curl -L https://github.com/broccolimicro/loom/releases/download/$TAG/lm-macos -o /usr/local/bin/lm
else
	curl -L https://github.com/broccolimicro/loom/releases/download/$TAG/lm-windows.zip -o lm-windows.zip
	unzip lm-windows.zip
fi

