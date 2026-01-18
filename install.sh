#!/bin/sh

set -e
TEMP_DIR=""
cleanup() {
	[ -n "$TEMP_DIR" ] && rm -rf "$TEMP_DIR"
}
trap cleanup EXIT

TAG=$(curl -Ls "https://api.github.com/repos/broccolimicro/loom/releases?per_page=1" | grep "\"name\": *\"v" | sed 's/.*: *"\(v[^"]*\)".*/\1/g')
OS=$(uname)

if [ "$OS" = "Linux" ]; then
	TEMP_DIR="$(mktemp -d)"
	DEB_URL="https://github.com/broccolimicro/loom/releases/download/$TAG/lm-linux.deb"

	echo "Downloading installation files..."
	if curl -L --progress-bar --retry 3 --retry-delay 2 "$DEB_URL" -o "$TEMP_DIR/lm-linux.deb"; then
	  echo "Download successful."
	else
	  echo "Failed to download tarball. Please check the URL or your internet connection."
	  exit 1
	fi

	echo "Installing package..."
	echo "The following commands will be run with elevated privileges (sudo):"
	echo ""
	echo "  dpkg -i $TEMP_DIR/lm-linux.deb"
	echo ""
	while true; do
		read -r -p "Continue? [Y/n] " response </dev/tty
		case "$response" in
			[Yy]|'' ) break ;;	 # Default to yes if empty
			[Nn] ) echo "Installation cancelled."; exit 1 ;;
			* ) echo "Please enter Y or n." ;;
		esac
	done
	sudo dpkg -i "$TEMP_DIR/lm-linux.deb"
	
	# Post-install message
	echo "Installation complete!"
	echo "You can now use the 'lm' command."
elif [ "$OS" = "Darwin" ]; then
	# Define target directories
	BIN_DIR="/usr/local/bin"
	SHARE_DIR="/usr/local/share"
	TEMP_DIR="$(mktemp -d)"
	TARBALL_URL="https://github.com/broccolimicro/loom/releases/download/$TAG/lm-macos.tar.gz"

	echo "Downloading installation files..."
	if curl -L --progress-bar --retry 3 --retry-delay 2 "$TARBALL_URL" -o "$TEMP_DIR/lm-macos.tar.gz"; then
	  echo "Download successful."
	else
	  echo "Failed to download tarball. Please check the URL or your internet connection."
	  exit 1
	fi

	echo "Extracting files..."
	if tar -xzf "$TEMP_DIR/lm-macos.tar.gz" -C "$TEMP_DIR"; then
	  echo "Extraction complete."
	else
	  echo "Failed to extract files. Exiting."
	  exit 1
	fi

	echo "Installing package..."
	echo "The following commands will be run with elevated privileges (sudo):"
	echo ""
	echo "  cp \"$TEMP_DIR/lm-macos/bin/lm\" \"$BIN_DIR\""
	echo "  cp -r \"$TEMP_DIR/lm-macos/share/tech\" \"$SHARE_DIR\""
	echo "  chmod +x \"$BIN_DIR/lm\""
	echo "  chown -R root:staff \"$SHARE_DIR/tech\""
	echo "  chmod -R ug+rw \"$SHARE_DIR/tech\""
	echo ""
	while true; do
		read -r -p "Continue? [Y/n] " response
		case "$response" in
			[Yy]|'' ) break ;;	 # Default to yes if empty
			[Nn] ) echo "Installation cancelled."; exit 1 ;;
			* ) echo "Please enter Y or n." ;;
		esac
	done
	sudo sh -c "
		cp \"$TEMP_DIR/lm-macos/bin/lm\" \"$BIN_DIR\" &&
		cp -r \"$TEMP_DIR/lm-macos/share/tech\" \"$SHARE_DIR\" &&
		chmod +x \"$BIN_DIR/lm\" &&
		chown -R root:staff \"$SHARE_DIR/tech\" &&
		chmod -R ug+rw \"$SHARE_DIR/tech\"
	"
	echo "Binary installed successfully."

	# Post-install message
	echo "Installation complete!"
	echo "You can now use the 'lm' command."
else
	# Define target directories
	BIN_DIR="C:\\Program Files (x86)"
	TEMP_DIR="$(mktemp -d)"
	TARBALL_URL="https://github.com/broccolimicro/loom/releases/download/$TAG/lm-windows.zip"

	echo "Downloading installation files..."
	if curl -L --progress-bar --retry 3 --retry-delay 2 "$TARBALL_URL" -o "$TEMP_DIR/lm-windows.zip"; then
	  echo "Download successful."
	else
	  echo "Failed to download tarball. Please check the URL or your internet connection."
	  exit 1
	fi

	echo "Extracting files..."
	if unzip "$TEMP_DIR/lm-windows.zip" -d "$TEMP_DIR"; then
	  echo "Extraction complete."
	else
	  echo "Failed to extract files. Exiting."
	  exit 1
	fi

	# Install binary
	echo "Installing Loom to $BIN_DIR..."
	if cp -r "$TEMP_DIR/Loom" "$BIN_DIR/"; then
	  chmod +x "$BIN_DIR/Loom/bin/lm"
	  chmod -R ug+rw "$BIN_DIR/Loom/share/tech"
	  echo "Loom installed successfully."
	else
	  echo "Failed to install Loom. Please check permissions and try again."
	  exit 1
	fi

	# Post-install message
	echo "Installation complete!"
	echo "You can now use the 'lm' command."
fi

