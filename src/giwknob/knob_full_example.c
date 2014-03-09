/* knob_full_example.c  -  GiwKnob widget's full example
Copyright (C) 2006  Alexandre Pereira Bueno, Eduardo Parente Ribeiro

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Maintainers
Alexandre Pereira Bueno - alpebu@yahoo.com.br 
James Scott Jr <skoona@users.sourceforge.net>
*/

#include <gtk/gtk.h>
#include "giwknob.h" 

GtkWidget *window;
GtkWidget *grid1;

GtkWidget *frame_knob;
GtkWidget *vbox_knob;
GtkWidget *knob;
GtkWidget *spin_value;

GtkWidget *frame_configure;
GtkWidget *table_configure;
GtkWidget *label_minor;
GtkWidget *label_major;
GtkWidget *label_digits;
GtkWidget *spin_minor;
GtkWidget *spin_major;
GtkWidget *spin_digits;

GtkWidget *frame_adj;
GtkWidget *grid_adj;
GtkWidget *spin_adj_value;
GtkWidget *spin_adj_lower;
GtkWidget *spin_adj_upper;
GtkWidget *label_adj;
GtkWidget *label_adj_value;
GtkWidget *label_adj_lower;
GtkWidget *label_adj_upper;
GtkWidget *button_set_adj;

GtkWidget *frame_mouse;
GtkWidget *vbox_mouse;
GtkWidget *radio_disable;
GtkWidget *radio_automatic;
GtkWidget *radio_delayed;


void spin_value_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	// The spin and the knob has the same adjusment, so when the value is changed in the spin the value of the knob is changed too, because is the same memory adress. The giw_knob_set_value function is called only to redraw the widget
	giw_knob_set_value(GIW_KNOB(knob), gtk_spin_button_get_value(spinbutton));
}

void spin_major_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	// Setting the ticks number
	giw_knob_set_ticks_number(GIW_KNOB(knob), gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_major)), gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_minor)));
}

void spin_minor_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	// Setting the ticks number
	giw_knob_set_ticks_number(GIW_KNOB(knob), gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_major)), gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_minor)));
}

void spin_digits_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	// Setting the legends_digits number
	giw_knob_set_legends_digits(GIW_KNOB(knob), gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_digits)));
}

void button_set_adj_clicked(GtkButton *button, gpointer user_data)
{
	gint value, lower, upper;
	GObject *adj;
		
	// Taking the values of the spins (the range and value of the adjustment)
	value=gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_adj_value));
	lower=gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_adj_lower));
	upper=gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_adj_upper));
	
	if ((lower < upper) && (value >= lower) && (value <= upper) ) { // If there is not something weird!
		// Creating the new adjustment, and setting it to the knob
		adj=(GObject *) gtk_adjustment_new(value, lower, upper, 1, 1, 1);
		giw_knob_set_adjustment(GIW_KNOB(knob), GTK_ADJUSTMENT(adj));
		// Putting the adjustment in the spin too
		gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(spin_value), GTK_ADJUSTMENT(adj));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_value), gtk_adjustment_get_value(GTK_ADJUSTMENT(adj)));
	}
}

void radio_disable_toggled(GtkRadioButton *radiobutton, gpointer user_data)
{
	// Set mouse policy to disabled
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)))
		giw_knob_set_mouse_policy(GIW_KNOB(knob), GIW_KNOB_MOUSE_DISABLED);
}

void radio_automatic_toggled(GtkRadioButton *radiobutton, gpointer user_data)
{
	// Set mouse policy to update automatically
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)))
		giw_knob_set_mouse_policy(GIW_KNOB(knob), GIW_KNOB_MOUSE_AUTOMATICALLY);
}

void radio_disable_delayed(GtkRadioButton *radiobutton, gpointer user_data)
{
	// Set mouse policy to update delayed
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)))
		giw_knob_set_mouse_policy(GIW_KNOB(knob), GIW_KNOB_MOUSE_DELAYED);
}

int main (int argc, char *argv[])
{
	gtk_init (&argc, &argv);

	// Creating the window that will hold all the other widgets
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  
	gtk_window_set_title (GTK_WINDOW (window), "GiwKnob Full Example");
  
	g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (gtk_main_quit), NULL);
  
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);

	// Table with for cells to hold the four frames
	grid1=gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(window), grid1);
	gtk_widget_show(grid1);
  
	// The frame with the knob and the spin with his value
	frame_knob=gtk_frame_new("Knob:");
	gtk_grid_attach(GTK_GRID(grid1), frame_knob, 0, 0, 1, 1);
	gtk_widget_show(frame_knob);
	
	vbox_knob=gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(frame_knob), vbox_knob);
	gtk_widget_show(vbox_knob);
	
	// The knob, created with a convenience function wich doesn't need a gtkadjustment, it creates one with the range in the parameters.
	knob=giw_knob_new_with_adjustment(100.0, 100.0, 200.0);
	gtk_box_pack_start(GTK_BOX(vbox_knob), knob, TRUE, TRUE, 0);
	gtk_widget_show(knob);

	spin_value = gtk_spin_button_new_with_range(0, 200, 0.000001);
	gtk_box_pack_start(GTK_BOX(vbox_knob), spin_value, TRUE, FALSE, 0);
	gtk_widget_show (spin_value);
		
	// The frame with the spin to set the ticks and legends digits number
	frame_configure=gtk_frame_new("Configure:");
	gtk_grid_attach(GTK_GRID(grid1), frame_configure, 0, 1, 1, 1);
	gtk_widget_show(frame_configure);
	
	table_configure=gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(frame_configure), table_configure);
	gtk_widget_show(table_configure);
  
	label_minor=gtk_label_new("Minor Ticks:");
	gtk_grid_attach(GTK_GRID(table_configure), label_minor, 0, 0, 1, 1);
	gtk_widget_show (label_minor);
  
	spin_minor = gtk_spin_button_new_with_range(0, 50, 1);
	gtk_grid_attach(GTK_GRID(table_configure), spin_minor, 1, 0, 1, 1);
	gtk_widget_show (spin_minor);
  
	label_major=gtk_label_new("Major Ticks:");
	gtk_grid_attach(GTK_GRID(table_configure), label_major, 0, 1, 1, 1);
	gtk_widget_show (label_major);
  
	spin_major = gtk_spin_button_new_with_range(0, 50, 1);
	gtk_grid_attach(GTK_GRID(table_configure), spin_major, 1, 1, 1, 1);
	gtk_widget_show (spin_major);
	
	label_digits=gtk_label_new("Legends Digits:");
	gtk_grid_attach(GTK_GRID(table_configure), label_digits, 0, 2, 1, 1);
	gtk_widget_show (label_digits);
  
	spin_digits = gtk_spin_button_new_with_range(0, 50, 1);
	gtk_grid_attach(GTK_GRID(table_configure), spin_digits, 1, 2, 1, 1);
	gtk_widget_show (spin_digits);

	// Frame with the labels and spins that will set a new adjusment (range) for the knob	
	frame_adj=gtk_frame_new("New adjustment:");
	gtk_grid_attach(GTK_GRID(grid1), frame_adj, 1, 0, 1, 1);
	gtk_widget_show(frame_adj);
	
	grid_adj=gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(frame_adj), grid_adj);
	gtk_widget_show(grid_adj);
		
	label_adj_value = gtk_label_new("Value:");
	gtk_grid_attach(GTK_GRID(grid_adj), label_adj_value, 0, 0, 1, 1);
	gtk_widget_show (label_adj_value);
  
	spin_adj_value = gtk_spin_button_new_with_range(-1000000, 1000000, 1);
	gtk_grid_attach(GTK_GRID(grid_adj), spin_adj_value, 1, 0, 1, 1);
	gtk_widget_show (spin_adj_value);
  
	label_adj_lower = gtk_label_new("Lower:");
	gtk_grid_attach(GTK_GRID(grid_adj), label_adj_lower, 0, 1, 1, 1);
	gtk_widget_show (label_adj_lower);
  
	spin_adj_lower = gtk_spin_button_new_with_range(-1000000, 1000000, 1);
	gtk_grid_attach(GTK_GRID(grid_adj), spin_adj_lower, 1, 1, 1, 1);
	gtk_widget_show (spin_adj_lower);
  
	label_adj_upper = gtk_label_new("Upper:");
	gtk_grid_attach(GTK_GRID(grid_adj), label_adj_upper, 0, 2, 1, 1);
	gtk_widget_show (label_adj_upper);
  
	spin_adj_upper = gtk_spin_button_new_with_range(-1000000, 1000000, 1);
	gtk_grid_attach(GTK_GRID(grid_adj), spin_adj_upper, 1, 2, 1, 1);
	gtk_widget_show (spin_adj_upper);

	// The button, that, if pressed, will set the new adjusment
	button_set_adj = gtk_button_new_with_label("Set adjustment");
	gtk_grid_attach(GTK_GRID(grid_adj), button_set_adj, 0, 3, 2, 1);
	gtk_widget_show (button_set_adj);
	
	// Frame with the mouse options (radio buttons)
	frame_mouse=gtk_frame_new("Mouse policy:");
	gtk_grid_attach(GTK_GRID(grid1), frame_mouse, 1, 1, 1, 1);
	gtk_widget_show(frame_mouse);
	
	vbox_mouse=gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(frame_mouse), vbox_mouse);
	gtk_widget_show(vbox_mouse);

	radio_disable=gtk_radio_button_new_with_label(NULL, "Disabled");
	gtk_box_pack_start(GTK_BOX(vbox_mouse), radio_disable, TRUE, FALSE, 0);
	gtk_widget_show(radio_disable);
	
	radio_automatic=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_disable), "Update automatically");
	gtk_box_pack_start(GTK_BOX(vbox_mouse), radio_automatic, TRUE, FALSE, 0);
	gtk_widget_show(radio_automatic);
	
	radio_delayed=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_disable), "Update delayed");
	gtk_box_pack_start(GTK_BOX(vbox_mouse), radio_delayed, TRUE, FALSE, 0);
	gtk_widget_show(radio_delayed);
		
	
	// Connecting the signals
	g_signal_connect(G_OBJECT(spin_value), "value-changed", G_CALLBACK(spin_value_changed_value), NULL);
	g_signal_connect(G_OBJECT(spin_major), "value-changed", G_CALLBACK(spin_major_changed_value), NULL);
	g_signal_connect(G_OBJECT(spin_minor), "value-changed", G_CALLBACK(spin_minor_changed_value), NULL);
	g_signal_connect(G_OBJECT(spin_digits), "value-changed", G_CALLBACK(spin_digits_changed_value), NULL);
	g_signal_connect(G_OBJECT(button_set_adj), "clicked", G_CALLBACK(button_set_adj_clicked), NULL);
	
	g_signal_connect(G_OBJECT(radio_disable), "toggled", G_CALLBACK(radio_disable_toggled), NULL);
	g_signal_connect(G_OBJECT(radio_automatic), "toggled", G_CALLBACK(radio_automatic_toggled), NULL);
	g_signal_connect(G_OBJECT(radio_delayed), "toggled", G_CALLBACK(radio_disable_delayed), NULL);
	
	// Setting initial values
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_major), 5);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_minor), 3);
	giw_knob_set_ticks_number(GIW_KNOB(knob), 5, 3);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_digits), 3);
	giw_knob_set_legends_digits(GIW_KNOB(knob), 3);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_automatic), TRUE);
	
	// Seting the adjustment of te spin value to the same adjustment of the knob (so they have always the same range and value)
	gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(spin_value), giw_knob_get_adjustment(GIW_KNOB(knob)));
	
	giw_knob_set_title(GIW_KNOB(knob), "Choose the value");
	
	gtk_widget_show (window);
  
	gtk_main ();
  
	return 0;
}
