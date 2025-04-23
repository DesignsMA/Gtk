import sys
import gi
# At the VERY START of your script:
import os
os.environ["GTK_CSD"] = "0"
os.environ["GDK_BACKEND"] = "win32"
os.environ["GDK_RENDERER"] = "cairo"
gi.require_version('Gtk', '4.0')
gi.require_version('GLib', '2.0')
gi.require_version('Adw', '1')
from gi.repository import Gtk, Adw, GLib, Gdk
display = Gdk.Display.get_default()
print(display.__class__)  # Should show Win32Display
class MainWindow(Gtk.ApplicationWindow):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        
        # Disable Adwaita client side decorations to prevent black borders
        # gtk4 python binding (on windows) doesnt support transparency for the window in any way
        os.environ["GTK_CSD"] = "0"
        self.set_decorated(False)  # We'll implement our own controls
        
        # Window setup
        self.set_title("Custom Header Demo")
        self.set_default_size(650, 320)
        
        # Main container
        self.main_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.set_child(self.main_box)
        
        # 1. Create custom header bar
        self.header = Gtk.HeaderBar()
        self.main_box.append(self.header)
        
        # 2. Add window controls
        self.setup_custom_header()
        
        # 3. Add content
        self.content_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=10)
        self.content_box.set_margin_top(10)
        self.content_box.set_margin_bottom(10)
        self.content_box.set_margin_start(10)
        self.content_box.set_margin_end(10)
        self.main_box.append(self.content_box)
        
        # Your widgets
        self.btn1 = Gtk.Button(label="Hello")
        self.btn2 = Gtk.Button()
        self.content_box.append(self.btn1)
        self.content_box.append(self.btn2)
        self.btn1.connect('clicked', lambda btn: self.hello())
        
        # 4. Make header draggable
        self.setup_drag_gesture()
        
        # 5. Apply styling
        self.apply_styles()

    def setup_custom_header(self):
        # Left side elements
        menu_btn = Gtk.Button(icon_name="open-menu-symbolic")
        self.header.pack_start(menu_btn)
        
        # Title
        title = Gtk.Label(label="My App")
        title.get_style_context().add_class("title")
        self.header.set_title_widget(title)
        
        # Right side controls (minimize/maximize/close)
        btn_box = Gtk.Box(spacing=6)
        
        btn_min = Gtk.Button()
        btn_min.set_child(Gtk.Image(icon_name="window-minimize-symbolic"))
        btn_min.connect("clicked", lambda b: self.minimize())
        
        btn_max = Gtk.Button()
        btn_max.set_child(Gtk.Image(icon_name="window-maximize-symbolic"))
        btn_max.connect("clicked", lambda b: self.maximize())
        
        btn_close = Gtk.Button()
        btn_close.set_child(Gtk.Image(icon_name="window-close-symbolic"))
        btn_close.connect("clicked", lambda b: self.close())
        btn_close.get_style_context().add_class("close-btn")
        
        btn_box.append(btn_min)
        btn_box.append(btn_max)
        btn_box.append(btn_close)
        self.header.pack_end(btn_box)

    def setup_drag_gesture(self):
        drag = Gtk.GestureDrag()
        drag.set_button(1)  # Left mouse button
        drag.connect("drag-begin", self.on_drag_begin)
        drag.connect("drag-update", self.on_drag_update)
        self.header.add_controller(drag)

    def on_drag_begin(self, gesture, start_x, start_y):
        self.drag_start_pos = (start_x, start_y)

    def on_drag_update(self, gesture, offset_x, offset_y):
        if not hasattr(self, 'drag_start_pos'):
            return
        self.begin_move_drag(
            1,  # Left mouse button
            int(self.drag_start_pos[0] + offset_x),
            int(self.drag_start_pos[1] + offset_y),
            Gdk.CURRENT_TIME
        )

    def apply_styles(self):
        css = """
        .title {
            font-weight: bold;
            margin: 0 12px;
        }
        button {
            padding: 4px;
            min-width: 24px;
            min-height: 24px;
            margin: 2px;
        }
        .close-btn {
            color: white;
            background-color: #e74c3c;
        }
        .close-btn:hover {
            background-color: #c0392b;
        }
        window {
            border-radius: 40px
        }
        headerbar {
            background: @theme_bg_color;
            border-bottom: 1px solid @borders;
            padding: 4px;
        }
        """
        provider = Gtk.CssProvider()
        provider.load_from_data(css.encode())
        Gtk.StyleContext.add_provider_for_display(
            Gdk.Display.get_default(),
            provider,
            Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION
        )

    def minimize(self):
        self.minimize()

    def maximize(self):
        if self.is_maximized():
            self.unmaximize()
        else:
            self.maximize()

    def hello(self):
        self.btn2.set_label("Bye!")
        GLib.timeout_add(2000, self.bye)
    
    def bye(self):
        self.btn2.set_label('')

class MyApp(Adw.Application):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.connect('activate', self.on_activate)

    def on_activate(self, app):
        self.win = MainWindow(application=app)
        self.win.present()

app = MyApp(application_id="com.example.CustomHeaderApp")
app.run(sys.argv)