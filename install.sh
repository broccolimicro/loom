#!/bin/sh

TAG=$(curl -Ls "https://api.github.com/repos/broccolimicro/loom/releases?per_page=1" | grep "\"name\": *\"v" | sed 's/.*: *"\(v[^"]*\)".*/\1/g')
OS=$(uname)

if [ "$OS" = "Linux" ]; then
	curl -L https://github.com/broccolimicro/loom/releases/download/$TAG/lm-linux.deb -o lm-linux.deb
	sudo dpkg -i lm-linux.deb
elif [ "$OS" = "Darwin" ]; then
	# Define target directories
	BIN_DIR="/usr/local/bin"
	SHARE_DIR="/usr/local/share"
	TEMP_DIR="$(mktemp -d)"
	TARBALL_URL="https://github.com/broccolimicro/loom/releases/download/$TAG/lm-macos.tar.gz"

	# Check if the script is run with root permissions
	if [ "$(id -u)" -ne 0 ]; then
	  echo "This installer requires root privileges. Please run with sudo."
	  exit 1
	fi

	echo "Downloading installation files..."
	if curl -L "$TARBALL_URL" -o "$TEMP_DIR/lm-macos.tar.gz"; then
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

	# Ensure target directories exist
	echo "Creating necessary directories..."
	mkdir -p "$BIN_DIR" "$SHARE_DIR"

	# Install binary
	echo "Installing lm binary to $BIN_DIR..."
	if cp -r $TEMP_DIR/lm-macos/bin/lm "$BIN_DIR/"; then
	  chmod +x "$BIN_DIR/lm"
	  echo "Binary installed successfully."
	else
	  echo "Failed to install binary. Please check permissions and try again."
	  exit 1
	fi

	# Install shared resources
	echo "Installing shared resources to $SHARE_DIR..."
	if cp -r $TEMP_DIR/lm-macos/share/tech "$SHARE_DIR/"; then
	  chown -R root:staff "$SHARE_DIR/tech"
	  chmod -R ug+rw "$SHARE_DIR/tech"
	  echo "Shared resources installed successfully."
	else
	  echo "Failed to install shared resources. Please check permissions and try again."
	  exit 1
	fi

	rm -rf "$TEMP_DIR"

	# Post-install message
	echo "Installation complete!"
	echo "You can now use the 'lm' command."
else
	curl -L https://github.com/broccolimicro/loom/releases/download/$TAG/lm-windows.zip -o lm-windows.zip
	unzip lm-windows.zip
fi

