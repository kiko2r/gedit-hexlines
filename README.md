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

- gedit with Python plugin support (gedit 3.x and newer, including gedit 46+).
- PyGObject (usually installed together with gedit).

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
