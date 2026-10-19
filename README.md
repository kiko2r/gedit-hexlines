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

- gedit with Python plugin support: gedit 3.8 up to gedit 47.
- PyGObject **older than 3.52** (usually installed together with gedit).

You can check your PyGObject version with:

```bash
python3 -c "import gi; print(gi.__version__)"
```

### Newer distributions (GNOME 48 and later)

PyGObject 3.52 and later broke the Python plugin loader of gedit (libpeas 1),
and gedit 49 dropped Python plugins altogether. On these systems gedit reports
that the `python3` plugin loader could not be found, and **no** Python plugin
works. See the
[gedit announcement](https://gedit-text-editor.org/blog/2025-04-30-no-more-pythons-in-gedit.html).

This affects, among others, Fedora 42+, Arch Linux / Manjaro and recent Ubuntu
releases. Known to work: Ubuntu 24.04 LTS (gedit 46, PyGObject 3.48).

If you are on one of these systems you can:

- use the C version of the plugin, in the
  [`c-port`](https://github.com/kiko2r/gedit-hexlines/tree/c-port) branch, or
- use the [Flatpak version of gedit](https://flathub.org/apps/org.gnome.gedit),
  as long as it still ships an older PyGObject, installing the plugin in
  `~/.var/app/org.gnome.gedit/data/gedit/plugins` instead.

## Installation

### Using the install script

```bash
git clone https://github.com/kiko2r/gedit-hexlines.git
cd gedit-hexlines
./install.sh
```

The script copies `hexlines.plugin` and `hexlines.py` to
`~/.local/share/gedit/plugins`.

### Manual installation

```bash
mkdir -p ~/.local/share/gedit/plugins
cp hexlines.plugin hexlines.py ~/.local/share/gedit/plugins/
```

### Enabling the plugin

1. Restart gedit.
2. Open **Preferences → Plugins**.
3. Enable **Hexadecimal Line Numbers**.

You will probably want to turn off gedit's own line numbers
(**Preferences → View → Display line numbers**) so they aren't shown twice.

## Enabling the optional features

Both optional features are disabled by default. To enable one, open
`hexlines.py` and uncomment its line near the top of the file:

```python
IGNORE = None
# IGNORE = "#"   # <- uncomment to skip lines starting with "#"

BLOCK = None
# BLOCK = "@"    # <- uncomment to ignore blocks between "@" lines
```

You can also change `"#"` or `"@"` to any other prefix.

Then reinstall the plugin (`./install.sh`) and restart gedit.

## Uninstall

```bash
rm ~/.local/share/gedit/plugins/hexlines.plugin ~/.local/share/gedit/plugins/hexlines.py
```

## License

Released under the [MIT License](LICENSE).

Copyright (c) 2026 Francisco J. Romero Ramírez &lt;fj.romero@uco.es&gt;
