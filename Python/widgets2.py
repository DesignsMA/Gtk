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
        
        self.set_title("Widgets2")
        self.set_default_size(650,320)
        # Disable Adwaita client side decorations to prevent black borders
        # gtk4 python binding (on windows) doesnt support transparency for the window in any way
        os.environ["GTK_CSD"] = "0" #Use Windows native frame
        self.set_decorated(False)   #Disable ALL GTK decorations
        header = Gtk.HeaderBar()    #Create SINGLE custom header
        
        
        self.box1 = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.set_child(self.box1) # set box1 as a child of the main window
        self.box1.append(header) # since we append the title bar as a widget of the window it doesn't creates black borders
        
        self.box2 = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=100, halign=Gtk.Align.CENTER, homogeneous=True)
        self.box21 = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=3)
        self.box22 = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=3)
        
        self.box1.append(self.box2) # append container of widgets
        self.btn1 = Gtk.Button(label="Hello")
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
