#include<X11/Xlib.h>
#include<assert.h>
#include<math.h>
#include"GUI.h"
#include<stdio.h>
#define NIL (0)
LogoGUI::LogoGUI(int width, int height)
{
	this->height = height;
	this->width  = width;
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
	this->reset();
}

void LogoGUI::reset()
{
	this->x = this->width / 2;
	this->y = this->height / 2;
	this->pen = true;
	this->angle = 270;
	XClearWindow(this->dpy, this->win);
	XFlush(this->dpy);
}

void LogoGUI::setxy(double x, double y) {
	this->x = x;
	this->y = y;
}

void LogoGUI::fd(double len) {
	double ex, ey;
	double ang = this->angle * 0.0174533;
	ex = this->x + len * cos(ang);
	ey = this->y + len * sin(ang);
	printf("x=%f, y=%f, angle=%f\n", this->x, this->y, this->angle);
	if(this->pen) {
		XDrawLine(this->dpy, this->win, this->gc, this->x, this->y, ex, ey);
		XFlush(this->dpy);
	}
	this->x = ex;
	this->y = ey;

}

void LogoGUI::bd(double len) {
	this->fd(len*-1);	
}


void LogoGUI::rt(double angle) {
	this->angle += angle;
	if(this->angle >= 360) this->angle -= 360;
}

void LogoGUI::lt(double angle) {
	this->angle -= angle;
	if(this->angle < 0) this->angle += 360;
}

void LogoGUI::pu() {
	this->pen = false;
}

void LogoGUI::pd() {
	this->pen = true;
}
