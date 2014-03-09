/* knob_simple_example.c  -  GiwKnob simple example
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
GtkAdjustment *adjustment;
GtkWidget *vbox;
GtkWidget *knob;
GtkWidget *label;
char str[20];

void adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data)
{
	// Creating the string
	sprintf(str, "Value : %f", gtk_adjustment_get_value(adjustment)); 
	
	// Now, changing the text of the label
	gtk_label_set_text(GTK_LABEL(label), str); 
}

int main (int argc, char *argv[])
{
	gtk_init (&argc, &argv);

	// Creating the winow wich will hold all the other widgets
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  
	gtk_window_set_title (GTK_WINDOW (window), "GiwKnob Simple Example");
  
	g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (gtk_main_quit), NULL);
  
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);

	vbox=gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show(vbox);
  
	// The knob, created with a function (giw_knob_new_with_adjusment) wich doesn't need a adjustment
	knob = giw_knob_new_with_adjustment(100.0, 100.0, 200.0);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(knob), TRUE, TRUE, 0);
	gtk_widget_show(knob);
  
	// The labels 
	label=gtk_label_new("Click over the knob and move the mouse to change the value.");
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(label), TRUE, FALSE, FALSE);
	gtk_widget_show(label);
	
	label=gtk_label_new("Value : 100.000000");
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(label), TRUE, FALSE, FALSE);
	gtk_widget_show(label);
	
	// Getting the adjustment of the dial, and connecting it's value-changed signal, to change the label text when the value of the knob is changed
	adjustment=giw_knob_get_adjustment(GIW_KNOB(knob));
	g_signal_connect(G_OBJECT(adjustment), "value-changed", G_CALLBACK(adjustment_value_changed), NULL);
  
	gtk_widget_show (window);
  
	gtk_main ();
  
	return 0;
}
