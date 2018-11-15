#include<X11/Xlib.h>
#include<assert.h>
#include"GUI.h"
#define NIL (0)
LogoGUI::LogoGUI(int width, int height)
{
	this->height = height;
	this->width  = width;
	this->x = width / 2;
	this->y = height / 2;
	this->pen = true;
	this->angle = 0;

	// Open Display and alloc Window
	this->dpy = XOpenDisplay(NIL);
	assert(dpy);
	this->blackColor = BlackPixel(dpy, DefaultScreen(dpy));
	this->whiteColor = WhitePixel(dpy, DefaultScreen(dpy));
	this->win = XCreateSimpleWindow(this->dpy, DefaultRootWindow(this->dpy),
			0, 0, width, height, 0, this->blackColor, this->blackColor);
	XSelectInput(this->dpy, this->win, StructureNotifyMask);
	XMapWindow(this->dpy, this->win);
	this->gc = XCreateGC(this->dpy, this->win, 0, NIL);
	XSetForeground(this->dpy, this->gc, this->whiteColor);
	for(;;) {
		XEvent e;
		XNextEvent(this->dpy, &e);
		if(e.type == MapNotify)
			break;
	}
	XDrawLine(this->dpy, this->win, this->gc, 10, 60, 180, 20);
	XFlush(this->dpy);
}

void LogoGUI::setxy(int x, int y) {
	this->x = x;
	this->y = y;
}

void LogoGUI::fd(int len) {
}

void LogoGUI::bd(int len) {
}

void LogoGUI::rt(int angle) {
}

void LogoGUI::lt(int angle) {
}

void LogoGUI::pu() {
	this->pen = false;
}

void LogoGUI::pd() {
	this->pen = true;
}
