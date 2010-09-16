#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "button.h"
#include "render.h"
#include "fakekey.h"

#define TRUE 1
#define FALSE 0

int hit = -1;
unsigned int modifiers = 0;
Button *buttons;

int *rows;
double *sum_rel_size;

ButtonPrototype layouts[] = {
	// text, keysym, design, row, relative size
	{"Esc", XK_Escape, 1, 0, 1.5},
	{"F1", XK_F1, 1, 0, 1.0},
	{"F2", XK_F2, 1, 0, 1.0},
	{"F3", XK_F3, 1, 0, 1.0},
	{"F4", XK_F4, 1, 0, 1.0},
	{"F5", XK_F5, 1, 0, 1.0},
	{"F6", XK_F6, 1, 0, 1.0},
	{"F7", XK_F7, 1, 0, 1.0},
	{"F8", XK_F8, 1, 0, 1.0},
	{"F9", XK_F9, 1, 0, 1.0},
	{"F10", XK_F10, 1, 0, 1.0},
	{"F11", XK_F11, 1, 0, 1.0},
	{"F12", XK_F12, 1, 0, 1.0},
	{"Prt Scr", XK_Print, 1, 0, 1.7},
	{"Scrl Lck", XK_Scroll_Lock, 1, 0, 1.7},
	{"Pause", XK_Pause, 1, 0, 1.7},
	{"Insert", XK_Insert, 1, 0, 1.7},
	{"Delete", XK_Delete, 1, 0, 1.7},
	{"Home", XK_Home, 1, 0, 1.7},
	{"End", XK_End, 1, 0, 1.7},
	{"Pg Up", XK_Prior, 1, 0, 1.7},
	{"Pg Dn", XK_Next, 1, 0, 1.7},

	{"|", XK_bar, 0, 1, 1.0},
	{"1", -1, 0, 1, 1.0},
	{"2", -1, 0, 1, 1.0},
	{"3", -1, 0, 1, 1.0},
	{"4", -1, 0, 1, 1.0},
	{"5", -1, 0, 1, 1.0},
	{"6", -1, 0, 1, 1.0},
	{"7", -1, 0, 1, 1.0},
	{"8", -1, 0, 1, 1.0},
	{"9", -1, 0, 1, 1.0},
	{"0", -1, 0, 1, 1.0},
	{"+", XK_plus, 0, 1, 1.0},
	{"\\", XK_backslash, 0, 1, 1.0},
	{"Backspace", XK_BackSpace, 1, 1, 2.5},

	{"Tab", XK_Tab, 1, 2, 1.5},
	{"Q", -1, 0, 2, 1.0},
	{"W", -1, 0, 2, 1.0},
	{"E", -1, 0, 2, 1.0},
	{"R", -1, 0, 2, 1.0},
	{"T", -1, 0, 2, 1.0},
	{"Y", -1, 0, 2, 1.0},
	{"U", -1, 0, 2, 1.0},
	{"I", -1, 0, 2, 1.0},
	{"O", -1, 0, 2, 1.0},
	{"P", -1, 0, 2, 1.0},
	{"Å", XK_aring, 0, 2, 1.0},
	{"^", XK_dead_diaeresis, 0, 2, 1.0},
	{"Enter", XK_Return, 1, 2, 2.0},

	{"Caps", XK_Caps_Lock, 1, 3, 1.75},
	{"A", -1, 0, 3, 1.0},
	{"S", -1, 0, 3, 1.0},
	{"D", -1, 0, 3, 1.0},
	{"F", -1, 0, 3, 1.0},
	{"G", -1, 0, 3, 1.0},
	{"H", -1, 0, 3, 1.0},
	{"J", -1, 0, 3, 1.0},
	{"K", -1, 0, 3, 1.0},
	{"L", -1, 0, 3, 1.0},
	{"Ø", XK_oslash, 0, 3, 1.0},
	{"Æ", XK_ae, 0, 3, 1.0},
	{"'", XK_apostrophe, 0, 3, 1.0},

	{"Shift", XK_Shift_L, 1, 4, 2.0},
	{"<", XK_less, 0, 4, 1.0},
	{"Z", -1, 0, 4, 1.0},
	{"X", -1, 0, 4, 1.0},
	{"C", -1, 0, 4, 1.0},
	{"V", -1, 0, 4, 1.0},
	{"B", -1, 0, 4, 1.0},
	{"N", -1, 0, 4, 1.0},
	{"M", -1, 0, 4, 1.0},
	{",", XK_comma, 0, 4, 1.0},
	{".", XK_period, 0, 4, 1.0},
	{"-", XK_minus, 0, 4, 1.0},
	{"Shift", XK_Shift_R, 1, 4, 2.0},

	{"Ctrl", XK_Control_L, 1, 5, 1.0},
	{"Win", XK_Super_L, 1, 5, 1.0},
	{"Alt", XK_Alt_L, 1, 5, 1.0},
	{"Space", XK_space, 0, 5, 3.0},
	{"Alt Gr", XK_ISO_Level3_Shift, 1, 5, 1.0},
	{"Ctrl", XK_Control_R, 1, 5, 1.0},
	{"<", XK_Left, 1, 5, 0.5},
	{"^", XK_Up, 1, 5, 0.5},
	{"v", XK_Down, 1, 5, 0.5},
	{">", XK_Right, 1, 5, 0.5}
};


cmp_bl(const void *p1, const void *p2) {
	if (((ButtonPrototype *)p1)->row < ((ButtonPrototype *)p2)->row)
		return -1;
	else if (((ButtonPrototype *)p1)->row > ((ButtonPrototype *)p2)->row)
		return 1;
	else
		return 0;
}

int get_num_buttons() {
	return sizeof(layouts) / sizeof(*layouts);
}

int get_num_rows() {
	return layouts[get_num_buttons()-1].row+1;
}

int intersects(Button *button, int x, int y) {
	return button->x < x && button->x + button->w > x && button->y < y && button->y + button->h > y;
}

void release_synonym_button(Button *button, int keysym, int keysym1, int keysym2) {
	if ((keysym == keysym1 || keysym == keysym2) && (button->keysym == keysym1 || button->keysym == keysym2) && button->pushed)
		button->pushed = FALSE;
}

toggle_button(int keysym) {
	int i;

	for (i=0; i<get_num_buttons(); i++) {
		release_synonym_button(buttons+i, keysym, XK_Shift_L, XK_Shift_R);
		release_synonym_button(buttons+i, keysym, XK_Control_L, XK_Control_R);
		release_synonym_button(buttons+i, keysym, XK_Alt_L, XK_Alt_R);
	}
}

int button_hit(int x, int y) {
	int i;
	for (i=0; i<get_num_buttons(); i++) {
		Button *button = buttons + i;
		if (intersects(button, x, y)) {
			if (button->keysym == XK_Shift_L || button->keysym == XK_Shift_R) {
				modifiers ^= ShiftMask;
				button->pushed = !button->pushed;
			}
			else if (button->keysym == XK_Control_L || button->keysym == XK_Control_R) {
				modifiers ^= ControlMask;
				button->pushed = !button->pushed;
			}
			else if (button->keysym == XK_Alt_L || button->keysym == XK_Alt_R) {
				modifiers ^= Mod1Mask;
				button->pushed = !button->pushed;
			}
			else if (button->keysym == XK_Caps_Lock) {
				modifiers ^= LockMask;
				button->pushed = !button->pushed;
			}
			else if (button->keysym == XK_ISO_Level3_Shift) {
				modifiers ^= Mod5Mask;
				button->pushed = !button->pushed;
			}
			else {
				buttons[i].pushed = TRUE;

				hit = i;

				fake_key_send(button->keysym, TRUE, modifiers);

				// Release shift
				if (modifiers & ShiftMask) {
					modifiers ^= ShiftMask;
					toggle_button(XK_Shift_L);
				}
			}

			flip_buffers();

			return 1;
		}
	}

	return 0;
}

void button_release() {
	if (hit == -1)
		return;

	fake_key_send(buttons[hit].keysym, FALSE, modifiers);

	buttons[hit].pushed = FALSE;

	flip_buffers();

	hit = -1;
}

void check_mouse_exited_button(int x, int y) {
	Button *button = buttons + hit;

	if (!intersects(button, x, y))
		button_release();
}

int init_buttons() {
	int i;
	int n_rows;
	static int n_buttons = sizeof(layouts)/sizeof(*layouts);

	qsort(layouts, n_buttons, sizeof(*layouts), cmp_bl);
	n_rows = get_num_rows();

	if (n_rows > MAX_ROWS) {
		fprintf(stderr, "Too many rows\n");
		return -1;
	}

	rows = malloc(sizeof(*rows) * n_rows);
	sum_rel_size = malloc(sizeof(*sum_rel_size) * n_rows);
	buttons = malloc(sizeof(*buttons) * n_buttons);
	if (rows == NULL || buttons == NULL || sum_rel_size == NULL)
		return -1;

	for (i=0; i<n_rows; i++) {
		rows[i] = 0;
		sum_rel_size[i] = 0.0;
	}

	for (i=0; i<n_buttons; i++) {
		char buf[] = " ";
		if (layouts[i].row < 0) {
			fprintf(stderr, "Row < 0\n");
			return -1;
		}

		Button *button = buttons + i;
		ButtonPrototype *bl = layouts + i;

		rows[bl->row]++;
		sum_rel_size[bl->row] += bl->rel_size;

		button->rel_size = bl->rel_size;
		button->type = bl->type;
		button->keysym = (bl->keysym < 0) ? XStringToKeysym(bl->text) : bl->keysym;
		strcpy(button->text, bl->text);
		button->pushed = FALSE;
	}

	return 0;
}
