/* gedit plugin: hexadecimal line numbers in the left gutter. */

#include <string.h>

#include <gtksourceview/gtksource.h>
#include <libpeas/peas.h>
#include <gedit/gedit-view.h>
#include <gedit/gedit-view-activatable.h>

/* Renderer position in the gutter (same area as the regular line numbers). */
#define POSITION -30

/* Lines starting with this prefix are not numbered.
 * Disabled by default: replace NULL with "#" to enable it. */
static const gchar *const IGNORE = NULL;

/* Lines starting with this prefix open and close a block: neither they nor
 * the lines between one and the next are numbered.
 * Disabled by default: replace NULL with "@" to enable it. */
static const gchar *const BLOCK = NULL;

/* Number shown for lines that are not counted. */
#define NOT_COUNTED (-1)

/* ------------------------------------------------------------------------ */
/* Renderer: draws each line number in hexadecimal.                         */

#define HEX_LINES_TYPE_RENDERER (hex_lines_renderer_get_type ())
G_DECLARE_FINAL_TYPE (HexLinesRenderer, hex_lines_renderer,
                      HEX_LINES, RENDERER, GtkSourceGutterRendererText)

struct _HexLinesRenderer
{
	GtkSourceGutterRendererText parent;
	gint digits;
	GArray *numbers; /* gint for each line, or NOT_COUNTED */
};

G_DEFINE_DYNAMIC_TYPE (HexLinesRenderer, hex_lines_renderer,
                       GTK_SOURCE_TYPE_GUTTER_RENDERER_TEXT)

static gboolean
line_starts_with (GtkTextBuffer *buf, gint line, const gchar *prefix)
{
	GtkTextIter start, end;
	gchar *text;
	gboolean result;

	gtk_text_buffer_get_iter_at_line (buf, &start, line);
	end = start;
	if (!gtk_text_iter_ends_line (&end))
		gtk_text_iter_forward_to_line_end (&end);

	text = gtk_text_iter_get_slice (&start, &end);
	result = g_str_has_prefix (text, prefix);
	g_free (text);
	return result;
}

static void
hex_lines_renderer_update_width (HexLinesRenderer *self, gint last_number)
{
	gchar *hex, *markup;
	gint digits, width;

	hex = g_strdup_printf ("%X", last_number);
	digits = MAX (2, (gint) strlen (hex));
	g_free (hex);

	if (digits == self->digits)
		return;
	self->digits = digits;

	hex = g_strnfill (digits, 'F');
	markup = g_strdup_printf ("<b>%s</b> ", hex);
	gtk_source_gutter_renderer_text_measure_markup (
		GTK_SOURCE_GUTTER_RENDERER_TEXT (self), markup, &width, NULL);
	gtk_source_gutter_renderer_set_size (GTK_SOURCE_GUTTER_RENDERER (self), width);
	g_free (markup);
	g_free (hex);
}

static void
hex_lines_renderer_refresh (HexLinesRenderer *self, GtkTextBuffer *buf)
{
	gint lines = gtk_text_buffer_get_line_count (buf);
	gint n = 0; /* numbering starts at 0 */
	gboolean in_block = FALSE;
	gint line;

	g_array_set_size (self->numbers, lines);
	for (line = 0; line < lines; line++) {
		gint number;

		if (BLOCK != NULL && line_starts_with (buf, line, BLOCK)) {
			in_block = !in_block;
			number = NOT_COUNTED;
		} else if (in_block ||
		           (IGNORE != NULL && line_starts_with (buf, line, IGNORE))) {
			number = NOT_COUNTED;
		} else {
			number = n++;
		}
		g_array_index (self->numbers, gint, line) = number;
	}

	hex_lines_renderer_update_width (self, MAX (n - 1, 0));
	/* following lines may have been renumbered */
	gtk_source_gutter_renderer_queue_draw (GTK_SOURCE_GUTTER_RENDERER (self));
}

static void
hex_lines_renderer_query_data (GtkSourceGutterRenderer      *renderer,
                               GtkTextIter                  *start,
                               GtkTextIter                  *end,
                               GtkSourceGutterRendererState  state)
{
	HexLinesRenderer *self = HEX_LINES_RENDERER (renderer);
	GtkSourceGutterRendererText *text = GTK_SOURCE_GUTTER_RENDERER_TEXT (renderer);
	guint line = gtk_text_iter_get_line (start);
	gint number = line < self->numbers->len
		? g_array_index (self->numbers, gint, line)
		: NOT_COUNTED;
	gchar *str;

	if (number == NOT_COUNTED) {
		gtk_source_gutter_renderer_text_set_text (text, "", -1);
		return;
	}

	if (state & GTK_SOURCE_GUTTER_RENDERER_STATE_CURSOR) {
		str = g_strdup_printf ("<b>%X</b>", number); /* current line in bold */
		gtk_source_gutter_renderer_text_set_markup (text, str, -1);
	} else {
		str = g_strdup_printf ("%X", number);
		gtk_source_gutter_renderer_text_set_text (text, str, -1);
	}
	g_free (str);
}

static void
hex_lines_renderer_finalize (GObject *object)
{
	HexLinesRenderer *self = HEX_LINES_RENDERER (object);

	g_array_unref (self->numbers);
	G_OBJECT_CLASS (hex_lines_renderer_parent_class)->finalize (object);
}

static void
hex_lines_renderer_constructed (GObject *object)
{
	G_OBJECT_CLASS (hex_lines_renderer_parent_class)->constructed (object);

	/* Done here because construct properties would override it in init(). */
	gtk_source_gutter_renderer_set_alignment (GTK_SOURCE_GUTTER_RENDERER (object),
	                                          1.0, 0.5); /* right-aligned */
}

static void
hex_lines_renderer_class_init (HexLinesRendererClass *klass)
{
	G_OBJECT_CLASS (klass)->constructed = hex_lines_renderer_constructed;
	G_OBJECT_CLASS (klass)->finalize = hex_lines_renderer_finalize;
	GTK_SOURCE_GUTTER_RENDERER_CLASS (klass)->query_data = hex_lines_renderer_query_data;
}

static void
hex_lines_renderer_class_finalize (HexLinesRendererClass *klass)
{
}

static void
hex_lines_renderer_init (HexLinesRenderer *self)
{
	self->numbers = g_array_new (FALSE, TRUE, sizeof (gint));
}

/* ------------------------------------------------------------------------ */
/* Plugin: adds the renderer to each view.                                  */

#define HEX_LINES_TYPE_PLUGIN (hex_lines_plugin_get_type ())
G_DECLARE_FINAL_TYPE (HexLinesPlugin, hex_lines_plugin,
                      HEX_LINES, PLUGIN, PeasExtensionBase)

struct _HexLinesPlugin
{
	PeasExtensionBase parent;
	GeditView *view;
	HexLinesRenderer *renderer;
	gulong handler;
};

enum
{
	PROP_0,
	PROP_VIEW
};

static void gedit_view_activatable_iface_init (GeditViewActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (HexLinesPlugin, hex_lines_plugin,
                                PEAS_TYPE_EXTENSION_BASE, 0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (GEDIT_TYPE_VIEW_ACTIVATABLE,
                                                               gedit_view_activatable_iface_init))

static GtkSourceGutter *
get_gutter (HexLinesPlugin *self)
{
	return gtk_source_view_get_gutter (GTK_SOURCE_VIEW (self->view),
	                                   GTK_TEXT_WINDOW_LEFT);
}

static void
on_changed (GtkTextBuffer *buf, HexLinesPlugin *self)
{
	hex_lines_renderer_refresh (self->renderer, buf);
}

static void
hex_lines_plugin_activate (GeditViewActivatable *activatable)
{
	HexLinesPlugin *self = HEX_LINES_PLUGIN (activatable);
	GtkTextBuffer *buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->view));

	self->renderer = g_object_ref_sink (g_object_new (HEX_LINES_TYPE_RENDERER, NULL));
	gtk_source_gutter_insert (get_gutter (self),
	                          GTK_SOURCE_GUTTER_RENDERER (self->renderer),
	                          POSITION);

	hex_lines_renderer_refresh (self->renderer, buf);
	self->handler = g_signal_connect (buf, "changed", G_CALLBACK (on_changed), self);
}

static void
hex_lines_plugin_deactivate (GeditViewActivatable *activatable)
{
	HexLinesPlugin *self = HEX_LINES_PLUGIN (activatable);
	GtkTextBuffer *buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (self->view));

	g_signal_handler_disconnect (buf, self->handler);
	gtk_source_gutter_remove (get_gutter (self),
	                          GTK_SOURCE_GUTTER_RENDERER (self->renderer));
	g_clear_object (&self->renderer);
}

static void
hex_lines_plugin_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
	HexLinesPlugin *self = HEX_LINES_PLUGIN (object);

	switch (prop_id) {
	case PROP_VIEW:
		g_set_object (&self->view, g_value_get_object (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
hex_lines_plugin_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
	HexLinesPlugin *self = HEX_LINES_PLUGIN (object);

	switch (prop_id) {
	case PROP_VIEW:
		g_value_set_object (value, self->view);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
hex_lines_plugin_dispose (GObject *object)
{
	HexLinesPlugin *self = HEX_LINES_PLUGIN (object);

	g_clear_object (&self->renderer);
	g_clear_object (&self->view);
	G_OBJECT_CLASS (hex_lines_plugin_parent_class)->dispose (object);
}

static void
hex_lines_plugin_class_init (HexLinesPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = hex_lines_plugin_set_property;
	object_class->get_property = hex_lines_plugin_get_property;
	object_class->dispose = hex_lines_plugin_dispose;

	g_object_class_override_property (object_class, PROP_VIEW, "view");
}

static void
hex_lines_plugin_class_finalize (HexLinesPluginClass *klass)
{
}

static void
hex_lines_plugin_init (HexLinesPlugin *self)
{
}

static void
gedit_view_activatable_iface_init (GeditViewActivatableInterface *iface)
{
	iface->activate = hex_lines_plugin_activate;
	iface->deactivate = hex_lines_plugin_deactivate;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	hex_lines_renderer_register_type (G_TYPE_MODULE (module));
	hex_lines_plugin_register_type (G_TYPE_MODULE (module));

	peas_object_module_register_extension_type (module,
	                                            GEDIT_TYPE_VIEW_ACTIVATABLE,
	                                            HEX_LINES_TYPE_PLUGIN);
}
