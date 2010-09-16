#include <stdlib.h>
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <cairo.h>
#include <cairo-xlib.h>

#include "main.h"
#include "button.h"
#include "render.h"
#include "fakekey.h"


/* TODO:
 * Tell window manager that window is dragged when overriding dragging_mouse
 * Button position bug on MAC
 * Support multiple buttons of the same type
 * Cache context??
 * http://www.handhelds.org/moin/moin.cgi/GeneratingSyntheticX11Events
 */


int disable_decorations = FALSE;
Atom del_msg;


void get_pointer_pos(Display *dis, Window win, int *x, int *y) {
	Window root_win;
	Window child_win;
	int root_x, root_y;
	unsigned int mask;

	XQueryPointer(dis, win, &root_win, &child_win, x, y, &root_x, &root_y, &mask);
}


void set_wm_hints(Display *dis, Window win) {
	Atom xa;
	XWMHints *wm_hints;
	Atom protocol;


	if (disable_decorations) {
		xa = XInternAtom(dis, "_NET_WM_WINDOW_TYPE", FALSE);
		if (xa != None) {
			Atom prop = XInternAtom(dis, "_NET_WM_WINDOW_TYPE_SPLASH", FALSE);
			//Atom prop = XInternAtom(dis, "_NET_WM_WINDOW_TYPE_DOCK", FALSE);
			//Atom prop = XInternAtom(dis, "_NET_WM_WINDOW_TYPE_UTILITY", FALSE);
			//Atom prop = XInternAtom(dis, "_NET_WM_WINDOW_TYPE_TOOLBAR", FALSE);
			//Atom prop = XInternAtom(dis, "_NET_WM_WINDOW_TYPE_NORMAL", FALSE);

			XChangeProperty(dis, win, xa, XA_ATOM, 32,
				PropModeReplace, (unsigned char *) &prop, 1);
		}
	}


	protocol = XInternAtom(dis, "WM_TAKE_FOCUS", FALSE);
	if (protocol != None)
		XSetWMProtocols(dis, win, &protocol, sizeof(protocol));


	xa = XInternAtom(dis, "_NET_WM_STATE", FALSE);
	if (xa != None) {
		Atom xa_prop = XInternAtom(dis, "_NET_WM_STATE_ABOVE", FALSE);

		XChangeProperty(dis, win, xa, XA_ATOM, 32,
			PropModeAppend, (unsigned char *) &xa_prop, 1);
	}

	if (!disable_decorations) {
		wm_hints = XAllocWMHints();
		if (wm_hints) {
			wm_hints->input = FALSE;
			//wm_hints->input = TRUE;
			wm_hints->flags = InputHint;
			XSetWMHints(dis, win, wm_hints);
			XFree(wm_hints);
		}
	}
}

void main_loop(Display *dis, Window win) {
	XEvent e;
	int running = TRUE;

	int dragging_mouse = FALSE;
	int holding_mouse = FALSE;
	int drag_x = 0, drag_y = 0;
	int new_x, new_y;

	while(running) {
		XNextEvent(dis, &e);
		switch (e.type) {
			case ClientMessage:
				if (e.xclient.data.l[0] == del_msg)
					running = FALSE;
				break;
			case Expose:
				if (e.xexpose.count < 1)
					flip_buffers();
				break;
			case ButtonPress:
				if (!button_hit(e.xbutton.x, e.xbutton.y)) {
					dragging_mouse = TRUE;
					drag_x = e.xbutton.x;
					drag_y = e.xbutton.y;
				}
				else
					holding_mouse = TRUE;

				break;
			case ButtonRelease:
				dragging_mouse = FALSE;
				holding_mouse = FALSE;

				button_release();

				break;
			case MotionNotify:
				if (holding_mouse)
					check_mouse_exited_button(e.xbutton.x, e.xbutton.y);
				else if (dragging_mouse && disable_decorations) {
					get_pointer_pos(dis, win, &new_x, &new_y);
					XMoveWindow(dis, win, new_x - drag_x, new_y - drag_y);
				}
				break;
			case ConfigureNotify:
				// Flush subsequent notifications
				while (XCheckTypedWindowEvent(dis, win, ConfigureNotify, &e)) {}

				if (!dragging_mouse) {
					resize_surface(e.xconfigure.width, e.xconfigure.height);
					while (XCheckTypedWindowEvent(dis, win, Expose, &e)) {}
				}
				break;
			default:
				break;
		}

		/*if (holding_mouse) {
			
		}*/
	}
}

int main() {
	Display *dis;
	Window win;
	int scr;

	int dis_width;

	dis = XOpenDisplay(NULL);
	if (dis == NULL) {
		fprintf(stderr, "Failed to open display\n");
		exit(EXIT_FAILURE);
	}

	scr = DefaultScreen(dis);

	dis_width = DisplayWidth(dis, scr);

	win = XCreateSimpleWindow(dis, RootWindow(dis, scr), 0, 0, dis_width*0.9, dis_width/4,
		0, BlackPixel(dis, scr), BlackPixel(dis, scr));

	XStoreName(dis, win, APP_NAME);

	XSelectInput(dis, win, ExposureMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask);

	set_wm_hints(dis, win);

	// Register interest in the delete window message
	del_msg = XInternAtom(dis, "WM_DELETE_WINDOW", FALSE);
	XSetWMProtocols(dis, win, &del_msg, 1);

	fake_key_init(dis);

	if (init_buttons() < 0)
		exit(EXIT_FAILURE);

	init_render(dis, win);

	// Paint first, then show window and draw main buffer
	paint();
	XMapWindow(dis, win);
	flip_buffers();


	// The main event loop
	main_loop(dis, win);


	finish_render();

	XDestroyWindow(dis, win);
	XCloseDisplay(dis);

	return 0;
}
