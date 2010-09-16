#ifndef BUTTON_H
#define BUTTON_H

#define MAX_ROWS 10


/*#define MOD_SHIFT (1<<0)
#define MOD_ALT_GR (1<<1)
#define MOD_ALT (1<<2)
#define MOD_CONTROL (1<<3)*/


typedef struct {
	double x;
	double y;
	double w;
	double h;
	int pushed;
	char text[15];
	int keysym;
	int type;
	double rel_size;
} Button;

typedef struct {
	char text[15]; // Button face text
	int keysym; // X keysym, or -1 to generate keysym from text
	int type; // Design (0 or 1)
	int row; // Row number, starting with 0
	double rel_size; // Relative button width
} ButtonPrototype;

int init_buttons();
int button_hit(int, int);
void button_release();
void check_mouse_exited_button(int, int);
int get_num_rows();
int get_num_buttons();
int get_mod_keysym(Button *);

#endif
