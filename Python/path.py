import os
os.environ["GI_TYPELIB_PATH"] = r"D:\Users\sepma\Downloads\Data\C728\Git\Gtk\py313_env\Lib\girepository-1.0"  # Force path

import gi
gi.require_version("Gtk", "4.0")  # Will now find the typelib
from gi.repository import Gtk

win = Gtk.Window(title="Success!")
win.connect("destroy", Gtk.main_quit)
win.show_all()
Gtk.main()