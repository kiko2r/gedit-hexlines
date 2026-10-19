# gedit plugin: hexadecimal line numbers in the left gutter.

import re

import gi

# gedit 46+ uses "GtkSource 300"; older versions use 4 or 3.0.
for _v in ("300", "4", "3.0"):
    try:
        gi.require_version("GtkSource", _v)
        break
    except ValueError:
        pass

from gi.repository import GObject, Gtk, Gedit, GtkSource

# Renderer position in the gutter (same area as the regular line numbers).
POSITION = -30

# Lines starting with this prefix are not numbered.
# Disabled by default: uncomment the second line to enable it.
IGNORE = None
# IGNORE = "#"

# Lines starting with this prefix open and close a block: neither they nor
# the lines between one and the next are numbered.
# Disabled by default: uncomment the second line to enable it.
BLOCK = None
# BLOCK = "@"

# Line breaks recognized by GtkTextBuffer.
LINE_BREAK = re.compile(r"\r\n|\n|\r|\u2029")


class HexLinesRenderer(GtkSource.GutterRendererText):
    """Draws each line number in hexadecimal."""

    def __init__(self):
        super().__init__()
        self.set_alignment(1.0, 0.5)  # right-aligned
        self._digits = 0
        self._numbers = []  # number for each line, or None if not counted

    def refresh(self, buf):
        text = buf.get_text(buf.get_start_iter(), buf.get_end_iter(), True)
        numbers = []
        n = 0  # numbering starts at 0
        in_block = False
        for line in LINE_BREAK.split(text):
            if BLOCK and line.startswith(BLOCK):
                in_block = not in_block
                numbers.append(None)
            elif in_block or (IGNORE and line.startswith(IGNORE)):
                numbers.append(None)
            else:
                numbers.append(n)
                n += 1
        self._numbers = numbers
        self.update_width(max(n - 1, 0))
        self.queue_draw()  # following lines may have been renumbered

    def update_width(self, last_number):
        digits = max(2, len(f"{last_number:X}"))
        if digits != self._digits:
            self._digits = digits
            width, _height = self.measure_markup("<b>" + "F" * digits + "</b> ")
            self.set_size(width)

    def do_query_data(self, start, end, state):
        line = start.get_line()
        number = self._numbers[line] if line < len(self._numbers) else None
        if number is None:
            self.set_text("", -1)
            return
        text = f"{number:X}"
        if state & GtkSource.GutterRendererState.CURSOR:
            self.set_markup(f"<b>{text}</b>", -1)  # current line in bold
        else:
            self.set_text(text, -1)


class HexLineNumbersPlugin(GObject.Object, Gedit.ViewActivatable):
    __gtype_name__ = "HexLineNumbersPlugin"

    view = GObject.Property(type=Gedit.View)

    def do_activate(self):
        self._renderer = HexLinesRenderer()
        gutter = self.view.get_gutter(Gtk.TextWindowType.LEFT)
        gutter.insert(self._renderer, POSITION)

        buf = self.view.get_buffer()
        self._renderer.refresh(buf)
        self._handler = buf.connect("changed", self._on_changed)

    def do_deactivate(self):
        self.view.get_buffer().disconnect(self._handler)
        self.view.get_gutter(Gtk.TextWindowType.LEFT).remove(self._renderer)
        self._renderer = None

    def _on_changed(self, buf):
        self._renderer.refresh(buf)
