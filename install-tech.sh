#!/bin/sh
set -e

TEMP_DIR=""
cleanup() {
	[ -n "$TEMP_DIR" ] && rm -rf "$TEMP_DIR"
}
trap cleanup EXIT

OS=$(uname)
USER_NAME=$(id -un)

if [ "$OS" = "Linux" ]; then
	SHARE_DIR="/usr/local/share"

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

	echo "Installing tech..."
	echo "The following actions will be performed with sudo:"
	echo ""
	echo "  groupadd cad (if needed)"
	echo "  usermod -aG cad \"$USER_NAME\""
	echo "  mkdir -p \"$SHARE_DIR\""

	if [ "$OVERWRITE" = "1" ]; then
		echo "  rm -rf \"$SHARE_DIR/tech\""
		echo "  cp -r tech \"$SHARE_DIR\""
	elif [ ! -d "$SHARE_DIR/tech" ]; then
		echo "  cp -r tech \"$SHARE_DIR\""
	else
		echo "  (keeping existing \"$SHARE_DIR/tech\")"
	fi

	echo "  chown -R root:cad \"$SHARE_DIR/tech\""
	echo "  chmod -R ug+rw \"$SHARE_DIR/tech\""
	echo "  find \"$SHARE_DIR/tech\" -type d -exec chmod g+s {} \\;"
	echo ""

	while true; do
		read -r -p "Continue? [Y/n] " response </dev/tty
		case "$response" in
			[Yy]|'') break ;;
			[Nn]) echo "Installation cancelled."; exit 1 ;;
			*) echo "Please enter Y or n." ;;
		esac
	done

	sudo OVERWRITE="$OVERWRITE" USER_NAME="$USER_NAME" SHARE_DIR="$SHARE_DIR" sh <<EOF
set -e

if ! getent group cad >/dev/null; then
	groupadd cad
fi

usermod -aG cad "$USER_NAME"

mkdir -p "$SHARE_DIR"

if [ "\$OVERWRITE" = "1" ]; then
	rm -rf "$SHARE_DIR/tech"
	cp -r tech "$SHARE_DIR"
elif [ ! -d "$SHARE_DIR/tech" ]; then
	cp -r tech "$SHARE_DIR"
fi

chown -R root:cad "$SHARE_DIR/tech"
chmod -R ug+rw "$SHARE_DIR/tech"
find "$SHARE_DIR/tech" -type d -exec chmod g+s {} \;
EOF

	echo
	echo "Installation complete."
	echo
	echo "NOTE: '$USER_NAME' has been added to the 'cad' group."
	echo "You must log out and back in (or start a new login session)"
	echo "before the new group membership takes effect."

elif [ "$OS" = "Darwin" ]; then
	SHARE_DIR="/usr/local/share"

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

	echo "Installing tech..."
	echo "The following actions will be performed with sudo:"
	echo ""
	echo "  mkdir -p \"$SHARE_DIR\""

	if [ "$OVERWRITE" = "1" ]; then
		echo "  rm -rf \"$SHARE_DIR/tech\""
		echo "  cp -r tech \"$SHARE_DIR\""
	elif [ ! -d "$SHARE_DIR/tech" ]; then
		echo "  cp -r tech \"$SHARE_DIR\""
	else
		echo "  (keeping existing \"$SHARE_DIR/tech\")"
	fi

	echo "  chown -R root:staff \"$SHARE_DIR/tech\""
	echo "  chmod -R ug+rw \"$SHARE_DIR/tech\""
	echo "  find \"$SHARE_DIR/tech\" -type d -exec chmod g+s {} \\;"
	echo ""

	while true; do
		read -r -p "Continue? [Y/n] " response </dev/tty
		case "$response" in
			[Yy]|'') break ;;
			[Nn]) echo "Installation cancelled."; exit 1 ;;
			*) echo "Please enter Y or n." ;;
		esac
	done

	sudo OVERWRITE="$OVERWRITE" SHARE_DIR="$SHARE_DIR" sh <<EOF
set -e

mkdir -p "$SHARE_DIR"

if [ "\$OVERWRITE" = "1" ]; then
	rm -rf "$SHARE_DIR/tech"
	cp -r tech "$SHARE_DIR"
elif [ ! -d "$SHARE_DIR/tech" ]; then
	cp -r tech "$SHARE_DIR"
fi

chown -R root:staff "$SHARE_DIR/tech"
chmod -R ug+rw "$SHARE_DIR/tech"
find "$SHARE_DIR/tech" -type d -exec chmod g+s {} \;
EOF

	echo
	echo "Installation complete."
	echo
	echo "You may need to log out and back in before group membership takes effect."
else
	# Define target directories
	BIN_DIR="C:\\Program Files (x86)"

	# Install binary
	echo "Installing the tech directory to $BIN_DIR\\Loom\\share..."
	if mkdir -p "$BIN_DIR\\Loom\\share"; then
	  chmod -R ug+rw "$BIN_DIR\\Loom\\share\\tech"
	else
	  echo "Failed to install. Please check permissions and try again."
	  exit 1
	fi

	# Post-install message
	echo "Installation complete!"
fi


