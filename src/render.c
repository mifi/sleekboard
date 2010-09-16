#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <X11/Xlib.h>

#include <cairo.h>
#include <cairo-xlib.h>

#include "button.h"
#include "render.h"

#define TRUE 1
#define FALSE 0

cairo_surface_t *offscreen;
cairo_surface_t *screen = NULL;
cairo_surface_t *overlay;

int s_width;
int s_height;

extern Button *buttons;
extern int *rows;
extern double *sum_rel_size;

extern int disable_decorations;

int utf8_strlen(const char *s) {
	int i=0, j=0;

	while (s[i]) {
		if ((s[i] & 0xC0) != 0x80)
			j++;
		i++;
	}
	return j;
}

void init_render(Display *dis, Window win) {
	XWindowAttributes wa;

	XGetWindowAttributes(dis, win, &wa);
	s_width = wa.width;
	s_height = wa.height;
	offscreen = cairo_image_surface_create(0, s_width, s_height);
	screen = cairo_xlib_surface_create(dis, win, DefaultVisual(dis, 0), s_width, s_height);

	overlay = cairo_image_surface_create(0, s_width, s_height);
}

void resize_surface(int w, int h) {
	s_width = w;
	s_height = h;

	// Offscreen
	cairo_surface_destroy(offscreen);
	offscreen = cairo_image_surface_create(0, s_width, s_height);

	// Overlay
	cairo_surface_destroy(overlay);
	overlay = cairo_image_surface_create(0, s_width, s_height);

	// Screen
	cairo_xlib_surface_set_size(screen, s_width, s_height);

	paint();
	flip_buffers();
}

void finish_render() {
	cairo_surface_destroy(offscreen);
	cairo_surface_destroy(screen);
	cairo_surface_destroy(overlay);
}


void redraw_overlay() {
	cairo_t *c = cairo_create(overlay);

	cairo_save(c);
	cairo_set_operator (c, CAIRO_OPERATOR_CLEAR);
	cairo_paint(c);
	cairo_restore(c);

	int i, n_buttons = get_num_buttons();
	for (i=0; i<n_buttons; i++) {
		Button *button = buttons + i;
		if (button->pushed) {
			cairo_rectangle(c, button->x, button->y, button->w, button->h);
			if (button->type == 0)
				cairo_set_source_rgba(c, 0.0, 0.0, 0.0, 0.3);
			else
				cairo_set_source_rgba(c, 1.0, 1.0, 1.0, 0.3);
		}

		cairo_fill(c);
	}

	cairo_destroy(c);
}

void flip_buffers() {
	redraw_overlay();

	cairo_t *c = cairo_create(screen);

	cairo_set_source_surface(c, offscreen, 0, 0);
	cairo_paint(c);

	cairo_set_source_surface(c, overlay, 0, 0);
	cairo_paint(c);

	cairo_destroy(c);
}

static void draw_rounded_rect(cairo_t *c, double x, double y, double width, double height, double radius) {
	if (radius > height / 2 || radius > width / 2)
		radius = fmin(height / 2, width / 2);

	cairo_move_to(c, x, y + radius);
	cairo_arc(c, x + radius, y + radius, radius, M_PI, -M_PI / 2);
	cairo_line_to(c, x + width - radius, y);
	cairo_arc(c, x + width - radius, y + radius, radius, -M_PI / 2, 0);
	cairo_line_to(c, x + width, y + height - radius);
	cairo_arc(c, x + width - radius, y + height - radius, radius, 0, M_PI / 2);
	cairo_line_to(c, x + radius, y + height);
	cairo_arc(c, x + radius, y + height - radius, radius, M_PI / 2, M_PI);

	cairo_close_path(c);
}


void draw_button_text(cairo_t *c, Button *button) {
	double x = button->x;
	double y = button->y;
	double w = button->w;
	double h = button->h;
	const char *text = button->text;
	int type = button->type;
	int pushed = button->pushed;

	cairo_text_extents_t te;

	cairo_select_font_face(c, "", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

	if (utf8_strlen(text) == 1)
		cairo_set_font_size(c, fmin(w, h)*0.6);
	else
		cairo_set_font_size(c, fmin(w / strlen(text)*1.3, h*0.6));


	cairo_text_extents(c, text, &te);

	cairo_move_to(c, x + w/2 - (te.width / 2 + te.x_bearing), y + h/2 - (te.height / 2 + te.y_bearing));
	cairo_text_path(c, text);

	if (type == 0)
		cairo_set_source_rgb(c, 0.15, 0.15, 0.15);
	else
		cairo_set_source_rgb(c, 1.0, 1.0, 1.0);
	cairo_fill(c);
}

void draw_button(cairo_t *c, Button *button) {
	double x = button->x;
	double y = button->y;
	double w = button->w;
	double h = button->h;
	const char *text = button->text;
	int type = button->type;
	int pushed = button->pushed;

	double r = fmin(w, h)/12;
	double diagonal = sqrt(pow(w, 2) + pow(h, 2));
	double line_width = fmin(w, h)/12;

	// Shadow
	cairo_pattern_t *pat = cairo_pattern_create_radial(x+w/2, y+h*0.5, 0.0, x+w/2, y+h*0.5, diagonal*0.55);
	cairo_pattern_add_color_stop_rgba(pat, 0.0, 0.0, 0.0, 0.0, 0.7);
	cairo_pattern_add_color_stop_rgba(pat, 1.0, 0.0, 0.0, 0.0, 0.0);

	cairo_pattern_t *pat2 = cairo_pattern_create_linear(0.0, y, 0.0, y+h);
	cairo_pattern_add_color_stop_rgb(pat2, 1, 0.15, 0.15, 0.15);
	cairo_pattern_add_color_stop_rgb(pat2, 0, 0.6, 0.6, 0.6);

	cairo_pattern_t *pat3 = cairo_pattern_create_linear(0.0, y, 0.0, y+h);

	if (type == 0) {
		cairo_pattern_add_color_stop_rgb(pat3, 0, 1.0, 1.0, 1.0);
		cairo_pattern_add_color_stop_rgb(pat3, 0.47, 0.9, 0.9, 0.9);
		cairo_pattern_add_color_stop_rgb(pat3, 0.53, 0.8, 0.8, 0.8);
		cairo_pattern_add_color_stop_rgb(pat3, 1, 0.7, 0.7, 0.7);
	}
	else {
		cairo_pattern_add_color_stop_rgb(pat3, 0, 0.7, 0.7, 0.7);
		cairo_pattern_add_color_stop_rgb(pat3, 0.47, 0.6, 0.6, 0.6);
		cairo_pattern_add_color_stop_rgb(pat3, 0.53, 0.5, 0.5, 0.5);
		cairo_pattern_add_color_stop_rgb(pat3, 1, 0.4, 0.4, 0.4);	
	}

	cairo_set_line_width(c, line_width);

	// Shadow
	cairo_rectangle(c, x-w/2, y, 2*w, 1.5*h);
	cairo_set_source(c, pat);
	cairo_fill(c);

	// The button itself
	draw_rounded_rect(c, x, y, w, h, r);
	cairo_set_source(c, pat3);
	cairo_fill_preserve(c);

	cairo_set_source(c, pat2);
	cairo_stroke(c);


	cairo_pattern_destroy(pat);
	cairo_pattern_destroy(pat2);
	cairo_pattern_destroy(pat3);

	// Text on the button
	draw_button_text(c, button);
}

void draw_button_struct(Button *button) {
	cairo_t *c = cairo_create(offscreen);
	draw_button(c, button);
	cairo_destroy(c);
}

/*void build_cached_graphic() {
	int i, n_buttons = get_num_buttons();
	for (i=0; i<n_buttons; i++) {
		buttons[i].gfx = cairo_image_surface_create(0, s_width, s_height);
		
	}
}*/

void paint() {
	cairo_t *c = cairo_create(offscreen);

	// Background pattern
	cairo_pattern_t *pat1 = cairo_pattern_create_radial(s_width/2, s_height/2, s_width/16, s_width/2,  s_height/2, s_width/2);
	cairo_pattern_add_color_stop_rgb(pat1, 0.0, 0.6, 0.6, 0.6);
	cairo_pattern_add_color_stop_rgb(pat1, 1.0, 0.2, 0.2, 0.2);

	// Background
	cairo_rectangle(c, 0.0, 0.0, s_width, s_height);
	cairo_set_source(c, pat1);
	cairo_fill(c);
	cairo_pattern_destroy(pat1);

	int i, j, sum = 0, n_rows = get_num_rows();

	double margin_top;
	if (disable_decorations)
		margin_top = 25;
	else
		margin_top = 0;

	double margin = fmin(s_height-margin_top, s_width)/30;
	double padding_x = (fmin(s_width, s_height-margin_top) - 2*margin)/30;
	double padding_y = (fmin(s_width, s_height-margin_top) - 2*margin)/30;

	double y_at = margin;

	for (i=0; i<n_rows; i++) {
		double h = (s_height-margin_top - 2*margin - padding_y * (n_rows-1))/n_rows;

		int n_cols = rows[i];
		double x_at = margin;
		for (j=0; j<n_cols; j++) {
			Button *button = buttons + sum + j;

			double w = (s_width - 2*margin - padding_x * (n_cols-1)) / sum_rel_size[i] * button->rel_size;

			button->x = x_at;
			button->y = y_at + margin_top;
			button->w = w;
			button->h = h;

			draw_button(c, button);

			x_at += w + padding_x;
		}

		y_at += h + padding_y;

		sum += rows[i];
		if (sum > get_num_buttons())
			break;
	}


	// Border
	cairo_set_line_width(c, fmin(s_width, s_height)/60);

	cairo_move_to(c, 0, 0);
	cairo_line_to(c, s_width, 0);
	cairo_line_to(c, s_width, s_height);
	cairo_line_to(c, 0, s_height);
	cairo_line_to(c, 0, 0);
	cairo_close_path(c);

	cairo_set_source_rgb(c, 0.15, 0.15, 0.15);
	cairo_stroke(c);

	cairo_destroy(c);
}
