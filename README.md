# Hexadecimal Line Numbers for gedit

A gedit plugin that shows line numbers in **hexadecimal** in the left gutter.

It is useful when writing assembly, machine code listings, ROM/memory dumps or any
file where each line maps to an address or index.

## Features

- Line numbers in hexadecimal (`00`, `01`, … `0A`, `0B`, …), starting at **0**.
- The current line number is shown in **bold**.
- The gutter width adapts to the number of hex digits needed (minimum 2).

### Optional features (disabled by default)

- **Skipped lines**: lines starting with `#` are not numbered, so the
  following lines keep their numbering.
- **Ignored blocks**: a line starting with `@` opens a block and the next line
  starting with `@` closes it. Neither those lines nor anything between them is
  numbered.

With both enabled:

```
00  FFF1
    # this line is not numbered
    @
    anything in here is ignored
    @
01  CRA
02  ADD 0
```

See [Enabling the optional features](#enabling-the-optional-features).

## Requirements

- gedit 3.x or newer (tested with gedit 46).
- To build the plugin: a C compiler, `pkg-config` and the gedit development
  files. The install script tells you which packages to install if they are
  missing:

| Distribution     | Packages                                            |
|------------------|-----------------------------------------------------|
| Debian / Ubuntu  | `sudo apt install build-essential pkg-config gedit-dev` |
| Fedora           | `sudo dnf install gcc pkgconf gedit-devel`          |
| Arch / Manjaro   | `sudo pacman -S --needed base-devel gedit`          |
| openSUSE         | `sudo zypper install gcc pkg-config gedit-devel`    |

### Why C and not Python?

The plugin used to be written in Python, but PyGObject 3.52 and later broke
the Python plugin loader of gedit, and gedit 49 dropped Python plugins
altogether (gedit reports that the `python3` plugin loader could not be
found). See the
[gedit announcement](https://gedit-text-editor.org/blog/2025-04-30-no-more-pythons-in-gedit.html).

The Python version is kept in the [`python/`](python) folder. It still works on
gedit 3.8–47 with PyGObject older than 3.52, and in the
[Flatpak version of gedit](https://flathub.org/apps/org.gnome.gedit) as long
as it ships an older PyGObject.

## Installation

### Using the install script

```bash
git clone -b c-port https://github.com/kiko2r/gedit-hexlines.git
cd gedit-hexlines
./install.sh
```

The script compiles the plugin and copies `libhexlines.so` and
`hexlines.plugin` to `~/.local/share/gedit/plugins`. If gedit is installed as
a Flatpak, it also copies the Python version to
`~/.var/app/org.gnome.gedit/data/gedit/plugins`.

The plugin has to be compiled for the gedit version installed on each
machine, so run `./install.sh` again after upgrading gedit.

### System-wide installation (packagers)

```bash
meson setup build --prefix=/usr
meson compile -C build
sudo meson install -C build
```

### Enabling the plugin

1. Restart gedit.
2. Open **Preferences → Plugins**.
3. Enable **Hexadecimal Line Numbers**.

You will probably want to turn off gedit's own line numbers
(**Preferences → View → Display line numbers**) so they aren't shown twice.

## Enabling the optional features

Both optional features are disabled by default. To enable one, open
`hexlines.c` and replace `NULL` with the prefix near the top of the file:

```c
static const gchar *const IGNORE = "#";  /* skip lines starting with "#" */

static const gchar *const BLOCK = "@";   /* ignore blocks between "@" lines */
```

You can also use any other prefix instead of `"#"` or `"@"`.

Then reinstall the plugin (`./install.sh`) and restart gedit.

## Uninstall

```bash
rm ~/.local/share/gedit/plugins/hexlines.plugin ~/.local/share/gedit/plugins/libhexlines.so
```

## License

Released under the [MIT License](LICENSE).

Copyright (c) 2026 Francisco J. Romero Ramírez &lt;fj.romero@uco.es&gt;
