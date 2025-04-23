import sys
import gi
gi.require_version('Gtk', '4.0')
gi.require_version('Adw', '1')
from gi.repository import Gtk, Adw, Gdk, Gio
import os
class MainWindow(Gtk.ApplicationWindow):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.set_title("Transparent GTK 4 App")
        self.set_default_size(600, 400)

        # Apply custom style
        css = b"""
        window {
            background-color: #2e3440;
            border-radius: 16px;
            box-shadow: none;
            
        }

        """
        
        provider = Gtk.CssProvider()
        provider.load_from_data(css)
        Gtk.StyleContext.add_provider_for_display(
            Gdk.Display.get_default(),
            provider,
            Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION
        )

        # Add a box or content to visualize
        label = Gtk.Label(label="Hello with transparency!")
        box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        box.append(label)
        self.set_child(box)

class MyApp(Adw.Application):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.connect('activate', self.on_activate)

    def on_activate(self, app):
        self.win = MainWindow(application=app)
        self.win.present()

app = MyApp(application_id="com.example.GtkApplication")
app.run(sys.argv)