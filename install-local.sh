#!/bin/sh

set -e
TEMP_DIR=""
cleanup() {
	[ -n "$TEMP_DIR" ] && rm -rf "$TEMP_DIR"
}
trap cleanup EXIT

TAG=$(curl -Ls "https://api.github.com/repos/broccolimicro/loom/releases?per_page=1" | grep "\"name\": *\"v" | sed 's/.*: *"\(v[^"]*\)".*/\1/g')
OS=$(uname)
USER_NAME=$(id -un)

if [ "$OS" = "Linux" ]; then
	SHARE_DIR="/usr/local/share"

	echo "Installing package..."
	echo "The following commands will be run with elevated privileges (sudo):"
	echo ""
	echo "  dpkg -i lm-linux.deb"
	echo "  groupadd cad # if needed"
	echo "  usermod -aG cad \"$USER_NAME\""
	echo "  mkdir -p \"$SHARE_DIR\""
	echo "  chown -R root:cad \"$SHARE_DIR/tech\""
	echo "  chmod -R ug+rw \"$SHARE_DIR/tech\""
	echo "  find \"$SHARE_DIR/tech\" -type d -exec chmod g+s {} \\;"
	echo ""
	while true; do
		read -r -p "Continue? [Y/n] " response </dev/tty
		case "$response" in
			[Yy]|'' ) break ;;	 # Default to yes if empty
			[Nn] ) echo "Installation cancelled."; exit 1 ;;
			* ) echo "Please enter Y or n." ;;
		esac
	done
	sudo USER_NAME="$USER_NAME" SHARE_DIR="$SHARE_DIR" sh <<EOF
set -e

dpkg -i "lm-linux.deb"

if ! getent group cad >/dev/null; then
	groupadd cad
fi

usermod -aG cad "$USER_NAME"
chown -R root:cad "$SHARE_DIR/tech"
chmod -R ug+rw "$SHARE_DIR/tech"
find "$SHARE_DIR/tech" -type d -exec chmod g+s {} \;
EOF
	
	# Post-install message
	echo "Installation complete!"
	echo "NOTE: '$USER_NAME' has been added to the 'cad' group."
	echo "You must log out and back in (or start a new login session)"
	echo "before the new group membership takes effect."
	echo ""
	echo "After login, you can now use the 'lm' command."
elif [ "$OS" = "Darwin" ]; then
	# Define target directories
	BIN_DIR="/usr/local/bin"
	SHARE_DIR="/usr/local/share"
	TEMP_DIR="$(mktemp -d)"

	echo "Extracting files..."
	if tar -xzf "lm-macos.tar.gz" -C "$TEMP_DIR"; then
	  echo "Extraction complete."
	else
	  echo "Failed to extract files. Exiting."
	  exit 1
	fi

	OVERWRITE=0

	if [ -d "$SHARE_DIR/tech" ]; then
		echo "A technology directory already exists at:"
		echo "  $SHARE_DIR/tech"
		echo

		while true; do
			read -r -p "Overwrite it? [y/N] " response </dev/tty
			case "$response" in
				[Yy]) OVERWRITE=1; break ;;
				[Nn]|'') break ;;
				*) echo "Please enter y or N." ;;
			esac
		done
	fi

	echo "Installing package..."
	echo "The following actions will be performed with sudo:"
	echo ""
	echo "  cp \"$TEMP_DIR/lm-macos/bin/lm\" \"$BIN_DIR\""
	echo "  chmod +x \"$BIN_DIR/lm\""
	echo "  mkdir -p \"$SHARE_DIR\""

	if [ "$OVERWRITE" = "1" ]; then
		echo "  rm -rf \"$SHARE_DIR/tech\""
		echo "  cp -r \"$TEMP_DIR/lm-macos/share/tech\" \"$SHARE_DIR\""
	elif [ ! -d "$SHARE_DIR/tech" ]; then
		echo "  cp -r \"$TEMP_DIR/lm-macos/share/tech\" \"$SHARE_DIR\""
	else
		echo "  (keeping existing \"$SHARE_DIR/tech\")"
	fi

	echo "  chown -R root:staff \"$SHARE_DIR/tech\""
	echo "  chmod -R ug+rw \"$SHARE_DIR/tech\""
	echo "  find \"$SHARE_DIR/tech\" -type d -exec chmod g+s {} \\;"
	echo ""

	while true; do
		read -r -p "Continue? [Y/n] " response
		case "$response" in
			[Yy]|'' ) break ;;	 # Default to yes if empty
			[Nn] ) echo "Installation cancelled."; exit 1 ;;
			* ) echo "Please enter Y or n." ;;
		esac
	done

	sudo OVERWRITE="$OVERWRITE" USER_NAME="$USER_NAME" TEMP_DIR="$TEMP_DIR" SHARE_DIR="$SHARE_DIR" sh <<EOF
set -e

cp \"$TEMP_DIR/lm-macos/bin/lm\" \"$BIN_DIR\"
chmod +x \"$BIN_DIR/lm\"

mkdir -p "$SHARE_DIR"

if [ "\$OVERWRITE" = "1" ]; then
	rm -rf "$SHARE_DIR/tech"
	cp -r \"$TEMP_DIR/lm-macos/share/tech\" \"$SHARE_DIR\"
elif [ ! -d "$SHARE_DIR/tech" ]; then
	cp -r \"$TEMP_DIR/lm-macos/share/tech\" \"$SHARE_DIR\"
fi

chown -R root:staff "$SHARE_DIR/tech"
chmod -R ug+rw "$SHARE_DIR/tech"
find "$SHARE_DIR/tech" -type d -exec chmod g+s {} \;
EOF

	echo "Binary installed successfully."

	# Post-install message
	echo "Installation complete!"
	echo "You can now use the 'lm' command."
else
	# Define target directories
	BIN_DIR="C:\\Program Files (x86)"
	TEMP_DIR="$(mktemp -d)"

	echo "Extracting files..."
	if unzip "lm-windows.zip" -d "$TEMP_DIR"; then
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

