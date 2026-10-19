#!/bin/bash
# Builds the plugin and installs it for the current user.
# If gedit is installed as a Flatpak, the Python version is installed there too.
set -e
SRC="$(cd "$(dirname "$0")" && pwd)"
DEST="$HOME/.local/share/gedit/plugins"
FLATPAK_DEST="$HOME/.var/app/org.gnome.gedit/data/gedit/plugins"

# Prints the packages needed to build the plugin on this distribution.
print_build_deps() {
	. /etc/os-release 2>/dev/null || true
	case " $ID $ID_LIKE " in
	*" debian "* | *" ubuntu "*)
		echo "  sudo apt install build-essential pkg-config gedit-dev" ;;
	*" fedora "* | *" rhel "*)
		echo "  sudo dnf install gcc pkgconf gedit-devel" ;;
	*" arch "*)
		echo "  sudo pacman -S --needed base-devel gedit" ;;
	*" suse "* | *" opensuse "*)
		echo "  sudo zypper install gcc pkg-config gedit-devel" ;;
	*)
		echo "  a C compiler, pkg-config and the gedit development files" ;;
	esac
}

installed=0

if command -v gedit >/dev/null; then
	if ! command -v cc >/dev/null || ! command -v pkg-config >/dev/null \
	   || ! pkg-config --exists gedit; then
		echo "Cannot build the plugin. Install the build dependencies with:"
		print_build_deps
		exit 1
	fi

	BUILD="$(mktemp -d)"
	trap 'rm -rf "$BUILD"' EXIT
	# shellcheck disable=SC2046
	cc -shared -fPIC -O2 -o "$BUILD/libhexlines.so" "$SRC/hexlines.c" \
		$(pkg-config --cflags --libs gedit)

	mkdir -p "$DEST"
	# Remove a previous installation of the Python version.
	rm -rf "$DEST/hexlines.py" "$DEST/__pycache__"
	cp "$BUILD/libhexlines.so" "$SRC/hexlines.plugin" "$DEST/"
	echo "Plugin installed in $DEST"
	installed=1
fi

if command -v flatpak >/dev/null && flatpak info org.gnome.gedit >/dev/null 2>&1; then
	mkdir -p "$FLATPAK_DEST"
	cp "$SRC/python/hexlines.plugin" "$SRC/python/hexlines.py" "$FLATPAK_DEST/"
	echo "Python version installed for gedit Flatpak in $FLATPAK_DEST"
	installed=1
fi

if [ "$installed" = 0 ]; then
	echo "gedit was not found."
	exit 1
fi

echo "Now restart gedit, open Preferences > Plugins and enable 'Hexadecimal Line Numbers'."
