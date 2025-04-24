import gi
gi.require_version('Gtk', '4.0')
gi.require_version('WebKit2', '4.1')  # Use WebKit2 instead of WebKit
from gi.repository import Gtk, WebKit2  # Note: WebKit2, not WebKit

from pyvis.network import Network
import tempfile
import os

class GraphViewer(Gtk.Application):
    def __init__(self):
        super().__init__(application_id='org.example.graphviewer')
        
    def do_activate(self):
        # Create main window
        self.window = Gtk.ApplicationWindow(application=self)
        self.window.set_title("Interactive Graph Viewer")
        self.window.set_default_size(800, 600)
        
        # Create web view for displaying the PyVis graph
        self.webview = WebKit2.WebView()  # WebKit2.WebView instead of WebKit.WebView
        
        # Create a scrolled window
        scrolled = Gtk.ScrolledWindow()
        scrolled.set_child(self.webview)
        
        # Set up the window content
        self.window.set_child(scrolled)
        self.window.present()
        
        # Load and display the graph
        self.load_graph()
    
    def load_graph(self):
        """Create and display a sample interactive graph"""
        # Create a PyVis network
        net = Network(height="600px", width="100%", bgcolor="#222222", font_color="white")
        
        # Add sample nodes and edges (your preloaded data would go here)
        net.add_node(1, label="Node 1", title="Info about Node 1")
        net.add_node(2, label="Node 2", title="Info about Node 2")
        net.add_node(3, label="Node 3", title="Info about Node 3")
        net.add_node(4, label="Node 4", title="Info about Node 4")
        net.add_node(5, label="Node 5", title="Info about Node 5")
        
        net.add_edge(1, 2, weight=5)
        net.add_edge(1, 3, weight=3)
        net.add_edge(2, 4, weight=2)
        net.add_edge(3, 5, weight=7)
        net.add_edge(4, 5, weight=1)
        
        # Configure physics for interactivity
        net.toggle_physics(True)
        net.show_buttons(filter_=['physics'])
        
        # Save to a temporary HTML file
        temp_dir = tempfile.mkdtemp()
        html_file = os.path.join(temp_dir, "graph.html")
        net.save_graph(html_file)
        
        # Load the HTML into the web view
        self.webview.load_uri(f"file://{html_file}")

if __name__ == "__main__":
    app = GraphViewer()
    app.run(None)