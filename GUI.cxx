#include<X11/Xlib.h>
#define XK_MISCELLANY
#define XK_LATIN1
#include<X11/keysymdef.h>
#include<assert.h>
#include<math.h>
#include"GUI.h"
#include<stdio.h>
#include<string.h>
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
	// Load first font found
	int nfonts;
	char **fonts = XListFonts(this->dpy, "-*-fixed-*-*-*-*-12-*-*-*-*-*-*-*", 1, &nfonts);
	assert(nfonts==1);
	this->font = XLoadFont(this->dpy, fonts[0]);
	this->fontstruct = XLoadQueryFont(this->dpy, fonts[0]);
	XFreeFontNames(fonts);

	XSelectInput(this->dpy, this->win, StructureNotifyMask | KeyPressMask | ButtonPressMask | 
			ExposureMask);
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
	if(this->lines) delete this->lines;
	this->lines = new std::vector<int>();
	this->x = this->width / 2;
	this->y = (this->height - 100) / 2;
	this->pen = true;
	this->angle = 270;
	XClearWindow(this->dpy, this->win);
	this->drawInterface(NULL);
	XFlush(this->dpy);
}

void LogoGUI::drawLine(int x1, int y1, int x2, int y2)
{
	XDrawLine(this->dpy, this->win, this->gc, x1, y1, x2, y2);
}

void LogoGUI::drawInterface(char *c)
{
	XClearArea(this->dpy, this->win, 0 , 400, 400, 100, false);
	this->drawLine(0, 400, 400, 400);
	this->drawLine(0, 400, 0, 500);
	this->drawLine(400, 400, 400, 500);
	this->drawLine(0, 485, 400, 485);
	this->drawLine(0, 499, 400, 499);
	this->drawLine(370, 485, 370, 499);
	this->drawText(371, 497, "RUN");
	this->drawLog();
	this->drawCursor(c);
}

void LogoGUI::drawCursor(char *c)
{
	int x = 10;
	if(c)
	{
		x += XTextWidth(this->fontstruct, c, strlen(c));
	}
	this->drawLine(x, 488, x, 497);
}

void LogoGUI::logStr(char *c)
{
	if(this->nextLog >= 5) {
		free(this->log[0]);
		int i;
		for(i=1;i<5;i++)
			this->log[i-1] = this->log[i];
		this->nextLog = 4;
	}
	this->log[this->nextLog] = (char*) malloc(strlen(c) + 1);
	strcpy(this->log[this->nextLog], c);
	this->nextLog++;
}

void LogoGUI::drawLog()
{
	int i;
	for(i=0;i<this->nextLog;i++)
	{
		this->drawText(10, 418 + i * 15, this->log[i]);
	}
}

void LogoGUI::restore()
{
	int i;
	for(i=0; i < this->lines->size(); i+=4)
	{
		XDrawLine(this->dpy, this->win, this->gc, this->lines->at(i), this->lines->at(i+1),
			       this->lines->at(i+2), this->lines->at(i+3));
	}
}

void LogoGUI::readString(char *buf, int len)
{
	buf[0] = 0;
	XEvent e;
	XKeyPressedEvent *ke = (XKeyPressedEvent*) &e;
	XButtonEvent *be = (XButtonEvent*) &e;
	char c;
	KeySym ks;
	Status s;
	XIM xim = XOpenIM(this->dpy, NULL, "x-logo", "x-logo");
	assert(xim);
	XIC xic = XCreateIC(xim, XNInputStyle, 
			(XIMPreeditNothing | XIMStatusNothing),
                        XNClientWindow, this->win,
			XNFocusWindow, this->win, NULL);
	assert(xic);
	int i = 0;
	while(1)
	{
		XNextEvent(this->dpy, &e);
		if(e.type == KeyPress)
		{
		       	XmbLookupString(xic, ke, &c, 1, &ks, &s);
			if(ks==XK_Return) break;
			if(ks==XK_BackSpace) {
				if(i)
					buf[--i] = 0;
			} else {
				if(ks==XK_space) {
					c = ' ';
				}
				printf("Keycode: %i, keysym: %i, char: %c\n", ke->keycode, ks, c);
				buf[i] = c;
				buf[++i] = 0;
			}
		} else if(e.type == ButtonPress) {
			printf("ButtonPress");
			if(be->x > 370 && be->y > 485)
			{
				break;
			} else if(be->y < 493 && be->y > 405) {
				int index = (be->y - 405) / 15;
				if(this->nextLog > index && strlen(this->log[index]) < len) {
					strcpy(buf, this->log[index]);
					i = strlen(buf);
				}
			}
		} else if(e.type == Expose) {
			printf("Expose\n");
			this->restore();
		}
		this->drawInterface(buf);
		this->drawText(10, 497, buf);
		XFlush(this->dpy);
	}
	this->logStr(buf);
	XDestroyIC(xic);
	XCloseIM(xim);
}

void LogoGUI::drawText(int x, int y, const char *text)
{
	XTextItem t[1];
	t->chars = (char*) text;
	t->nchars = strlen(text);
	t->font = this->font;
	t->delta = 1;
	XDrawText(this->dpy, this->win, this->gc, x, y, t, 1);
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
		this->lines->push_back(this->x);
		this->lines->push_back(this->y);
		this->lines->push_back(ex);
		this->lines->push_back(ey);
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
