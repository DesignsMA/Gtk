import sys
import gi
gi.require_version('Gtk', '4.0')
gi.require_version('GLib', '2.0')
gi.require_version('Adw', '1')
from gi.repository import Gtk, Adw, GLib, Gdk, Gio # css
import os

class MainWindow(Gtk.ApplicationWindow):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        
        self.set_title("Widgets2")
        self.set_default_size(650,320)
        # Disable Adwaita client side decorations to prevent black borders
        # gtk4 python binding (on windows) doesnt support transparency for the window in any way
        os.environ["GTK_CSD"] = "0" #Use Windows native frame
        os.environ["GDK_BACKEND"] = "help" #Use Windows native frame
        self.set_decorated(False)   #Disable ALL GTK decorations
        self.header = Gtk.HeaderBar()    #Create SINGLE custom header
        
        
        self.box1 = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.set_child(self.box1) # set box1 as a child of the main window
        self.box1.append(self.header) # since we append the title bar as a widget of the window it doesn't creates black borders
        
        self.box2 = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=10, halign=Gtk.Align.CENTER)
        self.box21 = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=3)
        self.box22 = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=3)
        
        self.box1.append(self.box2) # append container of widgets
        self.btn1 = Gtk.Button(label="Hello")
        self.btn1.set_size_request(100,50)
        self.chk1 = Gtk.CheckButton(label="And goodbye?")
        self.box21.append(self.btn1) # adding btn1 to box
        self.box21.append(self.chk1) # adding btn2 to box
        self.box2.append(self.box21)
        self.box2.append(self.box22)
        # lambda functions or anonymous functions
        self.btn1.connect('clicked', lambda btn: self.hello()) #connect function to handle signal of type clicked
        
        # radial button
        self.radio1 = Gtk.CheckButton(label="test")
        self.radio2 = Gtk.CheckButton(label="test")
        self.radio3 = Gtk.CheckButton(label="test")
        self.radio2.set_group(self.radio1)
        self.radio3.set_group(self.radio1)
        
        self.box22.append(self.radio1)
        self.box22.append(self.radio2)
        self.box22.append(self.radio3)
        
        self.switch_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL)

        self.switch = Gtk.Switch()
        self.switch.set_active(True)  # Let's default it to on
        self.switch.connect("state-set", self.switch_switched) # Lets trigger a function

        self.switch_box.append(self.switch)
        self.label = Gtk.Label(label="A switch")
        self.switch_box.append(self.label)
        self.switch_box.set_spacing(5) # Add some spacing
        self.box22.append(self.switch_box) 
        
        self.slider = Gtk.Scale.new_with_range(
            orientation=Gtk.Orientation.HORIZONTAL,
            min=0.5,  # 50% of original size
            max=2.0,  # 200% of original size
            step=0.1
        )
        self.slider.set_digits(1)  # Number of decimal places to use
        self.slider.set_range(0, 2)
        self.slider.set_draw_value(True)  # Show a label with current value
        self.slider.set_value(1)  # Sets the current value/position
        self.slider.connect('value-changed', self.slider_changed)
        self.box22.append(self.slider)
        
        self.open_button = Gtk.Button(label="Open")
        self.open_button.set_icon_name("document-open-symbolic")
        self.header.pack_start(self.open_button)
        self.open_dialog = Gtk.FileDialog(modal=False)
        self.open_dialog.set_title("Select a File")
        self.open_button.connect('clicked', self.show_open_dialog)
        
        # MIME Type https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/MIME_types
        # Multipurpose internet mail extensions | type/subtype
        # FileFilter used to restrict the files being shown in a file chooser
        f = Gtk.FileFilter()
        f.set_name("Image files")
        f.add_mime_type("image/jpeg")
        f.add_mime_type("image/png")
        
    

        # GioListStore
        #  Used to store instances of gtk objects
        # Directly works with GTK widgets (ListView, DropDown, etc.)
        # Automatically notifies widgets when data changes
        # No manual refresh needed for UI updates
        # Type safe
        filters = Gio.ListStore.new(Gtk.FileFilter)  # Create a ListStore with the type Gtk.FileFilter
        # ensures that we add only FileFilter types
        # updates ui (dialog) instantly if we add another one, right now we are only using an image filter
        filters.append(f)  # Add the file filter to the ListStore
        
        self.open_dialog.set_filters(filters)  # Set the filters for the open dialog
        self.open_dialog.set_default_filter(f)
        self.loadCss()
        
        
    def show_open_dialog(self, button):
        self.open_dialog.open(self, None, self.open_dialog_open_callback)
        
    def open_dialog_open_callback(self, dialog, result):
        try:
            file = dialog.open_finish(result)
            if file is not None:
                print(f"File path is {file.get_path()}")
                # Handle loading file from here
        except GLib.Error as error:
            print(f"Error opening file: {error.message}")
     
    
    def loadCss(self):
        style = """
        /* Let's create a title class */
        .title {
            font-size: 25px;
            font-weight: bold;
        }
        window {
            background-color: #ffffff;
            color: #161616;
        }

        headerbar { 
            background-color: #ffffff;
            color: #161616;
        }
        
        button {
            transition: all 1000ms ease-out;
        }

        """
        css_provider = Gtk.CssProvider()
        css_provider.load_from_string(style)
        Gtk.StyleContext.add_provider_for_display(Gdk.Display.get_default(), css_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)
    def slider_changed(self, slider):
            GLib.timeout_add(200, lambda: self.btn1.set_size_request(100*slider.get_value(),50*slider.get_value() ))
            
            
            
    def switch_switched(self, switch, state):
        print(f"The switch has been switched {'on' if state else 'off'}")
        
    def hello(self):
        self.chk1.set_active(True)
        self.chk1.set_label('Bye bye!')
        GLib.timeout_add(2000,self.bye)
    
    def bye(self):
        self.chk1.set_active(False)
        self.chk1.set_label('And goodbye?')
        

class MyApp(Adw.Application):
    
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.connect('activate', self.on_activate)

    def on_activate(self, app):
        self.win = MainWindow(application=app)
        self.win.present()

app = MyApp(application_id="com.example.GtkApplication")
app.run(sys.argv)
