/* giwknob.c  -  GiwKnob widget's source    Version 0.1
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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "giwknob.h"

#define KNOB_DEFAULT_SIZE 150

/* Forward declarations */
static void giw_knob_class_init (GiwKnobClass * klass);
static void giw_knob_init (GiwKnob * knob);
static void giw_knob_finalize (GObject * object);
static void giw_knob_realize (GtkWidget * widget);
static void giw_knob_get_preferred_width (GtkWidget * widget,
                                          gint *
                                          minimal_width,
                                          gint *
                                          natural_width);
static void giw_knob_get_preferred_height (GtkWidget * widget,
                                           gint *
                                           minimal_height,
                                           gint *
                                           natural_height);
static void giw_knob_size_request (GtkWidget * widget,
                                   GtkRequisition *
                                   requisition);
static void giw_knob_size_allocate (GtkWidget * widget,
                                    GtkAllocation *
                                    allocation);
static gint giw_knob_draw (GtkWidget * widget, cairo_t * cr);
static gint giw_knob_button_press (GtkWidget * widget,
                                   GdkEventButton * event);
static gint giw_knob_button_release (GtkWidget * widget,
                                     GdkEventButton * event);
static gint giw_knob_motion_notify (GtkWidget * widget,
                                    GdkEventMotion * event);
static void giw_knob_style_set (GtkWidget * widget,
                                GtkStyle * previous_style);

/* Local data */
// Changes the value, by mouse position
void knob_update_mouse (GiwKnob * knob, gint x, gint y);

// Updates the false pointer's angle
void knob_update_false_mouse (GiwKnob * knob, gint x, gint y);

// Calculate the value, using the angle
gdouble knob_calculate_value_with_angle (GiwKnob * knob,
                                         gdouble angle);
// Calculate the angle, using the value
gdouble knob_calculate_angle_with_value (GiwKnob * knob,
                                         gdouble value);
// Calculate all sizes 
static void knob_calculate_sizes (GiwKnob * knob);

// To make the changes needed when someno changes the lower ans upper fields of the adjustment
static void giw_knob_adjustment_changed (GtkAdjustment *
                                         adjustment,
                                         gpointer data);
// To make the changes needed when someno changes the value of the adjustment
static void giw_knob_adjustment_value_changed (GtkAdjustment *
                                               adjustment,
                                               gpointer data);
// A not public knob_set_angle function, for internal using, it only sets the angle, nothing more
void knob_set_angle (GiwKnob * knob, gdouble angle);

// A not public knob_set_value function, for internal using, it only sets the value, nothing more
void knob_set_value (GiwKnob * knob, gdouble value);

// A function that creates the layout of legends and calculates it's sizes
void knob_build_legends (GiwKnob * knob);

// A function that frees the layout of legends 
void knob_free_legends (GiwKnob * knob);

// A function that creates the layout of the title
void knob_build_title (GiwKnob * knob);

// A function that calculates width and height of the legend's the layout
void knob_calculate_legends_sizes (GiwKnob * knob);

// A function that calculates width and height of the title's the layout
void knob_calculate_title_sizes (GiwKnob * knob);

/*********************
* Widget's Functions * 
*********************/

G_DEFINE_TYPE (GiwKnob, giw_knob, GTK_TYPE_WIDGET)

static void giw_knob_class_init (GiwKnobClass * class)
{
    GObjectClass *object_class;
    GtkWidgetClass *widget_class;

    object_class = G_OBJECT_CLASS (class);
    widget_class = (GtkWidgetClass *) class;

    object_class->finalize = giw_knob_finalize;

    widget_class->realize = giw_knob_realize;
    widget_class->draw = giw_knob_draw;
    widget_class->get_preferred_width =
        giw_knob_get_preferred_width;
    widget_class->get_preferred_height =
        giw_knob_get_preferred_height;
    widget_class->size_allocate = giw_knob_size_allocate;
    widget_class->button_press_event = giw_knob_button_press;
    widget_class->button_release_event =
        giw_knob_button_release;
    widget_class->motion_notify_event =
        giw_knob_motion_notify;
    widget_class->style_set = giw_knob_style_set;
}

static void
giw_knob_init (GiwKnob * knob)
{
    GtkStyleContext *context;

    g_return_if_fail (knob != NULL);
    g_return_if_fail (GIW_IS_KNOB (knob));

    knob->button = 0;
    knob->mouse_policy = GIW_KNOB_MOUSE_AUTOMATICALLY;
    knob->major_ticks = 9;
    knob->minor_ticks = 3;
    knob->major_ticks_size = 5;
    knob->minor_ticks_size = 3;
    knob->legends_digits = 3;
    knob->title = NULL;

    context =
        gtk_widget_get_style_context (GTK_WIDGET (knob));

    gtk_style_context_add_class (context,
                                 GTK_STYLE_CLASS_BUTTON);
}

GtkWidget *
giw_knob_new (GtkAdjustment * adjustment)
{
    GiwKnob *knob;
    gdouble value;

    g_return_val_if_fail (adjustment != NULL, NULL);

    knob = g_object_new (GIW_TYPE_KNOB, NULL);
    giw_knob_set_adjustment (knob, adjustment);

    g_object_get (knob->adjustment, "value", &value, NULL);

    // Without this, in the first draw, the pointer wouldn't be in the right value
    knob_set_angle (knob,
                    knob_calculate_angle_with_value (knob,
                                                     value));

    return GTK_WIDGET (knob);
}

GtkWidget *
giw_knob_new_with_adjustment (gdouble value,
                              gdouble lower, gdouble upper)
{
    GiwKnob *knob;
    gdouble l_value;

    knob = g_object_new (GIW_TYPE_KNOB, NULL);

    giw_knob_set_adjustment (knob, (GtkAdjustment *)
                             gtk_adjustment_new (value, lower,
                                                 upper, 1.0,
                                                 1.0, 1.0));

    g_object_get (knob->adjustment, "value", &l_value, NULL);
    // Without this, in the first draw, the pointer wouldn't be in the right value
    knob_set_angle (knob,
                    knob_calculate_angle_with_value (knob,
                                                     l_value));

    return GTK_WIDGET (knob);
}

static void giw_knob_finalize (GObject * object) {
    GiwKnob *knob;
    gint loop;

    knob = GIW_KNOB (object);

    if (knob->adjustment)
        g_object_unref (G_OBJECT (knob->adjustment));
    if (knob->legends) {
        for (loop = 0; loop < knob->major_ticks; loop++) {
            g_object_unref (G_OBJECT (knob->legends[loop]));
        }
        g_free (knob->legends);
    }
    if (knob->title_str)
        g_free (knob->title_str);
    if (knob->title)
        g_object_unref (G_OBJECT (knob->title));

    G_OBJECT_CLASS (giw_knob_parent_class)->finalize
        (object);
}

static void
giw_knob_realize (GtkWidget * widget)
{
    GiwKnob *knob;
    GdkWindowAttr attributes;
    GtkAllocation allocation;
    GdkWindow *window;
    gint attributes_mask;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GIW_IS_KNOB (widget));

    gtk_widget_set_realized (widget, TRUE);

    knob = GIW_KNOB (widget);

    gtk_widget_get_allocation (widget, &allocation);

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = allocation.x;
    attributes.y = allocation.y;
    attributes.width = allocation.width;
    attributes.height = allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.event_mask = gtk_widget_get_events (widget) |
        GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
        GDK_POINTER_MOTION_HINT_MASK;
    /*attributes.visual = gtk_widget_get_visual (widget); */
    /*attributes.colormap = gtk_widget_get_colormap (widget); */

    attributes_mask = GDK_WA_X | GDK_WA_Y;      //| GDK_WA_VISUAL;      // | GDK_WA_COLORMAP;

    /*window = gtk_widget_get_parent_window (widget); */
    window =
        gdk_window_new (gtk_widget_get_parent_window (widget),
                        &attributes, attributes_mask);

    gtk_widget_set_window (widget, window);
    g_object_ref (window);

    gtk_widget_register_window (widget, window);

    // Create the initial legends
    knob_build_legends (knob);
}

static void
giw_knob_size_request (GtkWidget * widget,
                       GtkRequisition * requisition)
{
    requisition->width = KNOB_DEFAULT_SIZE;
    requisition->height = KNOB_DEFAULT_SIZE;
}

static void
giw_knob_get_preferred_width (GtkWidget * widget,
                              gint * minimum_width,
                              gint * natural_width)
{
    GtkRequisition requisition;

    giw_knob_size_request (widget, &requisition);

    *minimum_width = *natural_width = requisition.width;
}

static void
giw_knob_get_preferred_height (GtkWidget * widget,
                               gint * minimum_height,
                               gint * natural_height)
{
    GtkRequisition requisition;

    giw_knob_size_request (widget, &requisition);

    *minimum_height = *natural_height = requisition.height;
}


static void
giw_knob_size_allocate (GtkWidget * widget,
                        GtkAllocation * allocation)
{
    GiwKnob *knob;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GIW_IS_KNOB (widget));
    g_return_if_fail (allocation != NULL);

    gtk_widget_set_allocation (widget, allocation);

    knob = GIW_KNOB (widget);

    if (gtk_widget_get_realized (widget)) {

        gdk_window_move_resize (gtk_widget_get_window
                                (widget), allocation->x,
                                allocation->y,
                                allocation->width,
                                allocation->height);

    }

    knob_calculate_sizes (knob);
}

static gint
giw_knob_draw (GtkWidget * widget, cairo_t * cr)
{
    GiwKnob *knob;
    GtkAllocation allocation;
    gdouble s, c;
    gint xc, yc;
    gdouble loop1;
    guint dx1, dy1, dx2, dy2;
    gint counter = 0;
    cairo_rectangle_int_t rect;

    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (GIW_IS_KNOB (widget), FALSE);

    knob = GIW_KNOB (widget);

    rect.x = 0;
    rect.y = 0;
    rect.width = gtk_widget_get_allocated_width (widget);
    rect.height = gtk_widget_get_allocated_height (widget);

    /*// Drawing backgorund */
    /*gtk_paint_flat_box (widget->style, */
    /*widget->window, */
    /*GTK_STATE_NORMAL, */
    /*GTK_SHADOW_NONE, */
    /*&rect, */
    /*widget, */
    /*NULL, */
    /*0, */
    /*0, */
    /*-1,*/
    /*-1);			*/

    /*// The arc */
    cairo_set_line_width (cr, 2);
    cairo_arc (cr, knob->x + ((knob->size / 2)),
               knob->y + (knob->size / 2), knob->radius, 0,
               2 * M_PI);
    cairo_stroke (cr);
    cairo_set_line_width (cr, 0.5);

    gtk_widget_get_allocation (widget, &allocation);

    /*// The center */
    xc = allocation.width / 2;
    yc = allocation.height / 2;

    /*// Draw the knob */
    s = sin (knob->angle);
    c = cos (knob->angle);

    cairo_move_to (cr, xc + c * ((float) knob->radius * 0.8),
                   yc - s * ((float) knob->radius * 0.8));
    cairo_line_to (cr, xc + c * knob->radius,
                   yc - s * knob->radius);
    cairo_stroke (cr);

    cairo_arc (cr, xc + c * ((float) knob->radius * 0.8),
               yc - s * ((float) knob->radius * 0.8),
               knob->radius * 0.2, 0, 2 * M_PI);
    cairo_stroke (cr);

    /*// Draw the false-pointer if the delayed policy of mouse is set and a button is pressed */
    if ((knob->mouse_policy ==
         GIW_KNOB_MOUSE_DELAYED) & (knob->button != 0)) {
        s = sin (knob->false_angle);
        c = cos (knob->false_angle);

        cairo_move_to (cr,
                       xc + c * ((float) knob->radius * 0.8),
                       yc - s * ((float) knob->radius * 0.8));
        cairo_line_to (cr, xc + c * knob->radius,
                       yc - s * knob->radius);

        cairo_arc (cr, xc + c * ((float) knob->radius * 0.8),
                   yc - s * ((float) knob->radius * 0.8),
                   knob->radius * 0.2, 0, 2 * M_PI);
        cairo_stroke (cr);
    }

    /*// Now, draw the ticks */
    /*// The major ticks (and legends) */
    cairo_set_line_width (cr, 1.5);
    if (knob->major_ticks != 0)
        for (loop1 = (3.0 * M_PI / 2.0); loop1 >= -0.0001; loop1 -= knob->d_major_ticks) {      // -0.0001 (and not 0) to avoid rounding errors 
            s = sin (loop1 - M_PI / 4.0);
            c = cos (loop1 - M_PI / 4.0);
            dx1 = c * knob->radius;
            dy1 = s * knob->radius;
            dx2 = c * (knob->radius + knob->major_ticks_size);
            dy2 = s * (knob->radius + knob->major_ticks_size);
            cairo_move_to (cr, xc + dx1, yc - dy1);
            cairo_line_to (cr, xc + dx2, yc - dy2);
            cairo_stroke (cr);
            // Drawing the legends 
            if (knob->legends_digits != 0)
                gtk_render_layout
                    (gtk_widget_get_style_context (widget),
                     cr,
                     xc + (c * knob->legend_radius) -
                     (knob->legend_width / 2),
                     yc - (s * knob->legend_radius) -
                     (knob->legend_height / 2),
                     knob->legends[counter]);
            counter++;
        }
    cairo_set_line_width (cr, 0.5);
    /*// The minor ticks */
    if (knob->minor_ticks != 0)
        for (loop1 = (3.0 * M_PI / 2.0); loop1 >= 0.0;
             loop1 -= knob->d_minor_ticks) {
            s = sin (loop1 - M_PI / 4.0);
            c = cos (loop1 - M_PI / 4.0);
            dx1 = c * knob->radius;
            dy1 = s * knob->radius;
            dx2 =
                c * (knob->radius +
                     2 * knob->minor_ticks_size);
            dy2 =
                s * (knob->radius +
                     2 * knob->minor_ticks_size);
            cairo_move_to (cr, xc + dx1, yc - dy1);
            cairo_line_to (cr, xc + dx2, yc - dy2);
            cairo_stroke (cr);
        }

    /*// Draw the title */
    if (knob->title_str != NULL)        // font_str==NULL means no title 
        gtk_render_layout (gtk_widget_get_style_context
                           (widget), cr,
                           xc - knob->title_width / 2,
                           knob->size - knob->title_height -
                           5, knob->title);

    return FALSE;
}

static gint
giw_knob_button_press (GtkWidget * widget,
                       GdkEventButton * event)
{
    GiwKnob *knob;
    gint xc, yc, dx, dy;
    GtkAllocation allocation;

    g_return_val_if_fail (widget != NULL, TRUE);
    g_return_val_if_fail (GIW_IS_KNOB (widget), TRUE);
    g_return_val_if_fail (event != NULL, TRUE);

    knob = GIW_KNOB (widget);

    if (knob->mouse_policy == GIW_KNOB_MOUSE_DISABLED)
        return TRUE;
    if (knob->button)
        return TRUE;            // Some button is already pressed

    /* To verify if the pointer is in the knob, the distance between the pointer and the center
       of the circle is calculated, if it's less the the radius of the circle , it's in!! */

    gtk_widget_get_allocation (widget, &allocation);

    xc = allocation.width / 2;
    yc = allocation.height / 2;

    dx = abs ((int) event->x - xc);
    dy = abs ((int) event->y - yc);

    if (!knob->button & (dx < knob->radius) &
        (dy < knob->radius))
        knob->button = event->button;

    return FALSE;
}

static gint
giw_knob_button_release (GtkWidget * widget,
                         GdkEventButton * event)
{
    GiwKnob *knob;
    gint x, y;

    g_return_val_if_fail (widget != NULL, TRUE);
    g_return_val_if_fail (GIW_IS_KNOB (widget), TRUE);
    g_return_val_if_fail (event != NULL, TRUE);

    knob = GIW_KNOB (widget);

    if (knob->mouse_policy == GIW_KNOB_MOUSE_DISABLED)
        return TRUE;

    // If the policy is delayed, now that the button was released (if it is), it's time to update the value
    if ((knob->mouse_policy == GIW_KNOB_MOUSE_DELAYED) &&
        (knob->button == event->button)) {
        x = event->x;
        y = event->y;

        knob_update_mouse (knob, x, y);
    }


    if (knob->button == event->button)
        knob->button = 0;

    return FALSE;
}

static gint
giw_knob_motion_notify (GtkWidget * widget,
                        GdkEventMotion * event)
{
    GiwKnob *knob;
    GdkWindow *window = gtk_widget_get_window (widget);
    GdkDevice *device;
    gint x, y;

    device = gtk_get_current_event_device ();

    g_return_val_if_fail (widget != NULL, TRUE);
    g_return_val_if_fail (GIW_IS_KNOB (widget), TRUE);
    g_return_val_if_fail (event != NULL, TRUE);

    knob = GIW_KNOB (widget);

    if (knob->mouse_policy == GIW_KNOB_MOUSE_DISABLED)
        return TRUE;

    // If the some button is pressed and the policy is set to update the value AUTOMATICALLY, update the knob's value 
    if ((knob->button != 0)
        && (knob->mouse_policy ==
            GIW_KNOB_MOUSE_AUTOMATICALLY)) {
        x = event->x;
        y = event->y;

        if (event->is_hint || (event->window != window)) {

            gdk_window_get_device_position (window, device,
                                            &x, &y, NULL);
        }

        knob_update_mouse (knob, x, y);
    }

    // If the some button is pressed and the policy is set to update the value delayed, update the knob's false pointer's angle 
    if ((knob->button != 0)
        && (knob->mouse_policy == GIW_KNOB_MOUSE_DELAYED)) {
        x = event->x;
        y = event->y;

        if (event->is_hint || (event->window != window)) {
            GdkDevice *device;

            gdk_window_get_device_position (window, device,
                                            &x, &y, NULL);
        }

        knob_update_false_mouse (knob, x, y);
    }

    /*if (knob->button != 0)
       {
       x = event->x;
       y = event->y;

       if (event->is_hint || (event->window != widget->window))
       gdk_window_get_pointer (widget->window, &x, &y, NULL);

       knob_update_mouse (knob, x,y);
       } */

    return FALSE;
}

static void
giw_knob_style_set (GtkWidget * widget,
                    GtkStyle * previous_style)
{
    GiwKnob *knob;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GIW_IS_KNOB (widget));

    knob = GIW_KNOB (widget);

    // The only thing to fo is recalculate the layout's sizes
    knob_calculate_legends_sizes (knob);
    knob_calculate_title_sizes (knob);
}

/******************
* Users Functions * 
******************/

gdouble
giw_knob_get_value (GiwKnob * knob)
{
    g_return_val_if_fail (knob != NULL, 0.0);
    g_return_val_if_fail (GIW_IS_KNOB (knob), 0.0);

    return (gtk_adjustment_get_value (knob->adjustment));
}

void
giw_knob_set_value (GiwKnob * knob, gdouble value)
{
    g_return_if_fail (knob != NULL);
    g_return_if_fail (GIW_IS_KNOB (knob));


    if (value != gtk_adjustment_get_value (knob->adjustment)) {
        knob_set_value (knob, value);
        gtk_adjustment_value_changed (knob->adjustment);
    }
}

void
giw_knob_set_adjustment (GiwKnob * knob,
                         GtkAdjustment * adjustment)
{
    g_return_if_fail (knob != NULL);
    g_return_if_fail (GIW_IS_KNOB (knob));
    g_return_if_fail (adjustment != NULL);

    // Freeing the last one
    if (knob->adjustment) {
        g_signal_handlers_disconnect_by_data (G_OBJECT
                                              (knob->adjustment),
                                              (gpointer)
                                              knob);
        g_object_unref (G_OBJECT (knob->adjustment));
    }

    knob->adjustment = adjustment;
    g_object_ref (G_OBJECT (knob->adjustment));

    g_signal_connect (G_OBJECT (adjustment), "changed",
                      G_CALLBACK
                      (giw_knob_adjustment_changed),
                      (gpointer) knob);
    g_signal_connect (G_OBJECT (adjustment), "value-changed",
                      G_CALLBACK
                      (giw_knob_adjustment_value_changed),
                      (gpointer) knob);

    gtk_adjustment_value_changed (adjustment);

    gtk_adjustment_changed (adjustment);
}

GtkAdjustment *
giw_knob_get_adjustment (GiwKnob * knob)
{
    g_return_val_if_fail (knob != NULL, NULL);
    g_return_val_if_fail (GIW_IS_KNOB (knob), NULL);

    return (knob->adjustment);
}

void
giw_knob_set_legends_digits (GiwKnob * knob,
                             guint digits_number)
{
    g_return_if_fail (knob != NULL);
    g_return_if_fail (GIW_IS_KNOB (knob));

    if (digits_number != knob->legends_digits) {
        knob_free_legends (knob);

        knob->legends_digits = digits_number;

        knob_build_legends (knob);
        knob_calculate_sizes (knob);
        gtk_widget_queue_draw (GTK_WIDGET (knob));
    }
}

void
giw_knob_set_ticks_number (GiwKnob * knob,
                           guint major, guint minor)
{
    g_return_if_fail (knob != NULL);
    g_return_if_fail (GIW_IS_KNOB (knob));

    if ((major != knob->major_ticks)
        || (minor != knob->minor_ticks)) {
        knob_free_legends (knob);

        knob->major_ticks = major;

        if (knob->major_ticks == 0)
            knob->minor_ticks = 0;      // It's impossible to have minor ticks without major ticks
        else
            knob->minor_ticks = minor;

        knob_build_legends (knob);
        knob_calculate_sizes (knob);
        gtk_widget_queue_draw (GTK_WIDGET (knob));
    }
}

void
giw_knob_set_mouse_policy (GiwKnob * knob,
                           GiwKnobMousePolicy policy)
{
    g_return_if_fail (knob != NULL);
    g_return_if_fail (GIW_IS_KNOB (knob));

    if (knob->button == 0)      // The policy can only be change when there is no button pressed
        knob->mouse_policy = policy;
}

static void
giw_knob_adjustment_changed (GtkAdjustment * adjustment,
                             gpointer data)
{
    GiwKnob *knob;

    g_return_if_fail (adjustment != NULL);
    g_return_if_fail (data != NULL);

    knob = GIW_KNOB (data);

    knob_free_legends (knob);
    knob_build_legends (knob);
    knob_calculate_sizes (knob);
    gtk_widget_queue_draw (GTK_WIDGET (knob));
}

static void
giw_knob_adjustment_value_changed (GtkAdjustment * adjustment,
                                   gpointer data)
{
    GiwKnob *knob;

    g_return_if_fail (adjustment != NULL);
    g_return_if_fail (data != NULL);

    knob = GIW_KNOB (data);

    knob_set_angle (knob,
                    knob_calculate_angle_with_value (knob,
                                                     gtk_adjustment_get_value
                                                     (adjustment)));

    gtk_widget_queue_draw (GTK_WIDGET (knob));
}

void
giw_knob_set_title (GiwKnob * knob, gchar * str)
{
    GtkWidget *widget;

    g_return_if_fail (knob != NULL);
    g_return_if_fail (GIW_IS_KNOB (knob));

    widget = GTK_WIDGET (knob);

    knob->title_str = g_strdup (str);   // Duplicate the string, after this, str can be freed

    knob_build_title (knob);
    knob_calculate_sizes (knob);
    gtk_widget_queue_draw (GTK_WIDGET (knob));
}

/******************
* Local Functions * 
******************/

void
knob_update_mouse (GiwKnob * knob, gint x, gint y)
{
    gint xc, yc;

    g_return_if_fail (knob != NULL);
    g_return_if_fail (GIW_IS_KNOB (knob));

    GtkAllocation allocation;

    gtk_widget_get_allocation (GTK_WIDGET (knob),
                               &allocation);

    xc = allocation.width / 2;
    yc = allocation.height / 2;

    // Calculating the new angle
    if (knob->angle != atan2 (yc - y, x - xc)) {
        knob_set_value (knob,
                        knob_calculate_value_with_angle (knob,
                                                         atan2
                                                         (yc -
                                                          y,
                                                          x -
                                                          xc)));
        gtk_adjustment_value_changed (knob->adjustment);
    }
}

void
knob_update_false_mouse (GiwKnob * knob, gint x, gint y)
{
    gint xc, yc;

    g_return_if_fail (knob != NULL);
    g_return_if_fail (GIW_IS_KNOB (knob));

    GtkAllocation allocation;

    gtk_widget_get_allocation (GTK_WIDGET (knob),
                               &allocation);

    xc = allocation.width / 2;
    yc = allocation.height / 2;

    // Calculating the new angle
    knob->false_angle = atan2 (yc - y, x - xc);

    // Putting the angle between 0 and 2PI, because the atan2 returns the angle between PI and -PI
    while (knob->false_angle < 0)
        knob->false_angle += (2.0 * M_PI);

    // Taking out of the "forbideen" region
    if ((knob->false_angle <= (3.0 * M_PI / 2.0)) &&
        (knob->false_angle > (5.0 * M_PI / 4.0)))
        knob->false_angle = 5.0 * M_PI / 4.0;
    if ((knob->false_angle < (7.0 * M_PI / 4.0)) &&
        (knob->false_angle >= (3.0 * M_PI / 2.0)))
        knob->false_angle = 7.0 * M_PI / 4.0;

    gtk_widget_queue_draw (GTK_WIDGET (knob));
}

static void
knob_calculate_sizes (GiwKnob * knob)
{
    GtkWidget *widget;
    GtkAllocation allocation;

    g_return_if_fail (knob != NULL);
    g_return_if_fail (GIW_IS_KNOB (knob));

    widget = GTK_WIDGET (knob);

    gtk_widget_get_allocation (widget, &allocation);

    // Getting the radius and size
    if (allocation.width < allocation.height) {
        knob->size = allocation.width;
        knob->x = 0;
        knob->y = allocation.height / 2 - knob->size / 2;
    }
    else {
        knob->size = allocation.height;
        knob->y = 0;
        knob->x = allocation.width / 2 - knob->size / 2;
    }

    // The distance between the radius and the widget limits is the bigger dimension of the legends plus the major_ticks_size, so it's the half of size, less the bigger dimension of the legends less the major_ticks size (wich depends of the radius), them, with some algebra, it results in this equation:
    knob->radius =
        8 * ((knob->size / 2) -
             sqrt (knob->legend_width * knob->legend_width +
                   knob->legend_height *
                   knob->legend_height)) / 9;

    knob->d_major_ticks =
        (3.0 * M_PI / 2.0) / (knob->major_ticks - 1);
    knob->d_minor_ticks =
        knob->d_major_ticks / (knob->minor_ticks + 1);

    knob->major_ticks_size = knob->radius / 8.0;
    knob->minor_ticks_size = knob->radius / 16.0;

    // The legend will in the middle of the inside (plus the major_ticks_size) and outside circle 
    knob->legend_radius =
        ((knob->radius + knob->major_ticks_size +
          (knob->size / 2)) / 2);
}

gdouble
knob_calculate_value_with_angle (GiwKnob * knob,
                                 gdouble angle)
{
    gdouble d_angle = 0.0;      // How many the pointer is far from the lower angle (5PI/4)

    g_return_val_if_fail (knob != NULL, 0.0);
    g_return_val_if_fail (GIW_IS_KNOB (knob), 0.0);

    // Putting the angle between 0 and 2PI, because the atan2 returns the angle between PI and -PI
    while (angle < 0)
        angle = angle + (2.0 * M_PI);

    // Taking out of the "forbideen" region
    if ((angle <= (3.0 * M_PI / 2.0))
        && (angle > (5.0 * M_PI / 4.0)))
        angle = 5.0 * M_PI / 4.0;
    if ((angle < (7.0 * M_PI / 4.0))
        && (angle >= (3.0 * M_PI / 2.0)))
        angle = 7.0 * M_PI / 4.0;

    // Calculating the distance (in radians) between the pointer and the lower angle   
    if (angle <= (5.0 * M_PI / 4.0))
        d_angle = (5.0 * M_PI / 4.0) - angle;
    if (angle >= (7.0 * M_PI / 4.0))
        d_angle = (13.0 * M_PI / 4.0) - angle;

    return (gtk_adjustment_get_lower (knob->adjustment) +
            fabs (gtk_adjustment_get_upper (knob->adjustment)
                  -
                  gtk_adjustment_get_lower (knob->adjustment))
            * d_angle / (3.0 * M_PI / 2.0));
}

gdouble
knob_calculate_angle_with_value (GiwKnob * knob,
                                 gdouble value)
{
    gdouble angle;

    g_return_val_if_fail (knob != NULL, 0.0);
    g_return_val_if_fail (GIW_IS_KNOB (knob), 0.0);

    angle =
        (value -
         gtk_adjustment_get_lower (knob->adjustment) * (3.0 *
                                                        M_PI /
                                                        2.0) /
         fabs (gtk_adjustment_get_upper (knob->adjustment) -
               gtk_adjustment_get_lower (knob->adjustment)));

    // Now, the angle is relative to the 3 o'clock position, and need to be changed in order to be ralative to the initial angle ((5.0*M_PI/4.0)
    angle = (5.0 * M_PI / 4.0) - angle;

    return (angle);
}

void
knob_set_angle (GiwKnob * knob, gdouble angle)
{

    g_return_if_fail (knob != NULL);
    g_return_if_fail (GIW_IS_KNOB (knob));

    // Putting the angle between 0 and 2PI(360�)
    while (angle > 2.0 * M_PI)
        angle = angle - (2.0 * M_PI);

    while (angle < 0)
        angle = angle + (2.0 * M_PI);

    if (knob->angle != angle) {
        // Taking out of the "forbideen" region
        if ((angle <= (3.0 * M_PI / 2.0))
            && (angle > (5.0 * M_PI / 4.0)))
            angle = 5.0 * M_PI / 4.0;
        if ((angle < (7.0 * M_PI / 4.0))
            && (angle >= (3.0 * M_PI / 2.0)))
            angle = 7.0 * M_PI / 4.0;

        knob->angle = angle;
    }
}

void
knob_set_value (GiwKnob * knob, gdouble value)
{
    g_return_if_fail (knob != NULL);
    g_return_if_fail (GIW_IS_KNOB (knob));

    gtk_adjustment_set_value (knob->adjustment, value);
}

void
knob_build_legends (GiwKnob * knob)
{
    GtkWidget *widget;
    gint loop;
    gchar *str;

    g_return_if_fail (knob != NULL);

    widget = GTK_WIDGET (knob);

    if (knob->major_ticks == 0) // Preventing from bugs
        return;

    // Creating the legend's layouts
    if (knob->legends_digits != 0) {
        knob->legends =
            g_new (PangoLayout *, knob->major_ticks);
        str = g_new (gchar, knob->legends_digits + 1);  // +1 for the '/0'
        for (loop = 0; loop < knob->major_ticks; loop++) {

            g_snprintf (str, knob->legends_digits + 1, "%f", gtk_adjustment_get_lower (knob->adjustment) + loop * (gtk_adjustment_get_upper (knob->adjustment) - gtk_adjustment_get_lower (knob->adjustment)) / (knob->major_ticks - 1)); // Creating the legends string

            knob->legends[loop] =
                gtk_widget_create_pango_layout (widget, str);
        }
        g_free (str);

        // Getting the size of the legends 
        knob_calculate_legends_sizes (knob);
    }
    else {                      // If there are no legends (0 digits), the size is the major ticks size (5)
        knob->legend_width = 0;
        knob->legend_height = 0;
    }
}

void
knob_free_legends (GiwKnob * knob)
{
    gint loop;

    g_return_if_fail (knob != NULL);

    if (knob->legends != NULL) {
        for (loop = 0; loop < knob->major_ticks; loop++)
            if (knob->legends[loop] != NULL)
                g_object_unref (G_OBJECT
                                (knob->legends[loop]));
        g_free (knob->legends);
        knob->legends = NULL;
    }
}

void
knob_build_title (GiwKnob * knob)
{
    GtkWidget *widget;

    g_return_if_fail (knob != NULL);

    widget = GTK_WIDGET (knob);

    if (knob->title_str == NULL)        // Return if there is no title (the layout will be keeped, but not drawed)
        return;

    if (knob->title)
        pango_layout_set_text (knob->title, knob->title_str,
                               strlen (knob->title_str));
    else                        // If the title hasn't been created yet..
        knob->title =
            gtk_widget_create_pango_layout (widget,
                                            knob->title_str);

    // Calculating new size
    knob_calculate_title_sizes (knob);
}

void
knob_calculate_legends_sizes (GiwKnob * knob)
{
    GtkWidget *widget;

    g_return_if_fail (knob != NULL);

    widget = GTK_WIDGET (knob);

    if (knob->legends != NULL) {
        PangoFontDescription *desc;

        gtk_style_context_get (gtk_widget_get_style_context
                               (widget),
                               GTK_STATE_FLAG_NORMAL, "font",
                               &desc, NULL);

        pango_layout_set_font_description (knob->legends[0],
                                           desc);
        pango_layout_get_size (knob->legends[0],
                               &(knob->legend_width),
                               &(knob->legend_height));
        knob->legend_width /= PANGO_SCALE;
        knob->legend_height /= PANGO_SCALE;
    }
}

void
knob_calculate_title_sizes (GiwKnob * knob)
{
    GtkWidget *widget;

    g_return_if_fail (knob != NULL);

    if (knob->title == NULL)
        return;

    widget = GTK_WIDGET (knob);

    PangoFontDescription *desc;

    gtk_style_context_get (gtk_widget_get_style_context
                           (widget),
                           GTK_STATE_FLAG_NORMAL, "font",
                           &desc, NULL);

    pango_layout_set_font_description (knob->title, desc);
    pango_layout_get_size (knob->title, &(knob->title_width),
                           &(knob->title_height));
    knob->title_width /= PANGO_SCALE;
    knob->title_height /= PANGO_SCALE;

    knob_calculate_sizes (knob);
}
