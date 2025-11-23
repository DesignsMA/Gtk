import sys
import gi
gi.require_version('Gtk', '4.0')
gi.require_version('GLib', '2.0')
gi.require_version('Adw', '1')
from gi.repository import Gtk, Adw, GLib
import os

class MainWindow(Gtk.ApplicationWindow):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.set_title("Widgets")
        self.set_default_size(650,320)
        # Disable Adwaita client side decorations to prevent black borders
        # gtk4 python binding (on windows) doesnt support transparency for the window in any way
        os.environ["GTK_CSD"] = "0" #Use Windows native frame
        self.set_decorated(False)   #Disable ALL GTK decorations
        header = Gtk.HeaderBar()    #Create SINGLE custom header
        # This forces native window decorations | more compatible
        self.box1 = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.set_child(self.box1) # set box1 as a child of the main window
        self.box1.append(header) # since we append the title bar as a widget of the window it doesn't creates black borders
        self.btn1 = Gtk.Button(label="Hello")
        self.btn2 = Gtk.Button()
        self.box1.append(self.btn1) # adding btn1 to box
        self.box1.append(self.btn2) # adding btn2 to box
        # lambda functions or anonymous functions
        self.btn1.connect('clicked', lambda btn: self.hello()) #connect function to handle signal of type clicked
        
        self.box1

    def hello(self):
        self.btn2.set_label("Bye!")
        GLib.timeout_add(2000,self.bye)
    
    def bye(self):
        self.btn2.set_label('')
        

class MyApp(Adw.Application):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.connect('activate', self.on_activate)

    def on_activate(self, app):
        self.win = MainWindow(application=app)
        self.win.present()

app = MyApp(application_id="com.example.GtkApplication")
app.run(sys.argv)
