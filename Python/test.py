import gi

gi.require_version("Gdk", "4.0")
gi.require_version("Gtk", "4.0")

from gi.repository import Gdk
from gi.repository import Gtk


CSS = b"""
#toplevel {
    background-color: rgba(0, 255, 255, 0.5);
}
"""

provider = Gtk.CssProvider()
provider.load_from_data(CSS)
Gtk.StyleContext.add_provider_for_display(
    Gdk.Display.get_default(),
    provider,
    Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION
)
button1 = Gtk.Button(label="Hello, world!")
button1.set_margin_bottom(50)
button1.set_margin_top(50)


window = Gtk.Window(title="Hello World", name="toplevel")
screen = window.get_display()
visual = screen.gdk_display_get_name()
print(visual)
#window.set_visual(visual)
#window.add(box)
window.show_all()
window.connect("destroy", Gtk.main_quit)

Gtk.main()