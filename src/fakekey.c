#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "fakekey.h"

#define TRUE 1
#define FALSE 0


Window root_win;
Display *dis = NULL;


static XKeyEvent createKeyEvent(Display *dis, Window win, Window winRoot, int press, int keysym, unsigned int modifiers) {
   XKeyEvent event;

   event.display     = dis;
   event.window      = win;
   event.root        = winRoot;
   event.subwindow   = None;
   event.time        = CurrentTime;
   event.x           = 1;
   event.y           = 1;
   event.x_root      = 1;
   event.y_root      = 1;
   event.same_screen = TRUE;
   event.type        = press ? KeyPress : KeyRelease;
   event.keycode     = XKeysymToKeycode(dis, keysym);
   event.state       = modifiers;


   return event;
}

void fake_key_init(Display *d) {
	dis = d;
	root_win = XDefaultRootWindow(dis);
}

void fake_key_send(int keysym, int press, unsigned int modifiers) {
	int revert;
	Window focus_win;
	XKeyEvent event;

	XGetInputFocus(dis, &focus_win, &revert);

	// Button pressed/released
	if (press) {
		event = createKeyEvent(dis, focus_win, root_win, TRUE, keysym, modifiers);
		XSendEvent(event.display, event.window, TRUE, KeyPressMask, (XEvent *)&event);
	}
	else {
		event = createKeyEvent(dis, focus_win, root_win, FALSE, keysym, modifiers);
		XSendEvent(event.display, event.window, TRUE, KeyPressMask, (XEvent *)&event);
	}
}
