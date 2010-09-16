#ifndef RENDER_H
#define RENDER_H

#define SIZEX 640
#define SIZEY 480

void init_render(Display *, Window);
void finish_render();
void flip_buffers();
void draw_button_struct(Button *);
void paint();

#endif
