#include<X11/Xlib.h>
#include<X11/Xutil.h>
#define XK_MISCELLANY
#define XK_LATIN1
#include<X11/keysymdef.h>
#include<assert.h>
#include<math.h>
#include"GUI.h"
#include<stdio.h>
#include<string.h>
#define NIL (0)
// The main GUI component
// also does the turtle graphics,
// input, command history


// Creates window on screen and setup X11
LogoGUI::LogoGUI(int width, int height)
{
	this->height = height;
	this->width  = width;
	this->sayTxts = NULL;
	// Open Display and alloc Window
	this->dpy = XOpenDisplay(NIL);
	assert(dpy);
	this->blackColor = BlackPixel(dpy, DefaultScreen(dpy));
	this->whiteColor = WhitePixel(dpy, DefaultScreen(dpy));
	this->win = XCreateSimpleWindow(this->dpy, DefaultRootWindow(this->dpy),
			0, 0, width, height, 0, this->blackColor, this->blackColor);

	// Make window not resizeable
	XSizeHints hints;
	XGetWMNormalHints(this->dpy, this->win, &hints, NULL);
	hints.flags |= PMinSize | PMaxSize;
	hints.min_width = width;
	hints.max_width = width;
	hints.min_height = height;
	hints.max_height = height;
	XSetWMNormalHints(this->dpy, this->win, &hints);

	// Load first font found
	int nfonts;
	char **fonts = XListFonts(this->dpy, "-*-fixed-*-*-*-*-12-*-*-*-*-*-*-*", 1, &nfonts);
	assert(nfonts==1);
	this->fontstruct = XLoadQueryFont(this->dpy, fonts[0]);
	if(!this->fontstruct)
	{
		XFreeFontNames(fonts);
		fonts = XListFonts(this->dpy, "-*-fixed-*-*-*-*-*-*-*-*-*-*-*-*", 1, &nfonts);
		assert(nfonts==1);
		this->fontstruct = XLoadQueryFont(this->dpy, fonts[0]);
		assert(this->fontstruct);
		

	}
	this->font = XLoadFont(this->dpy, fonts[0]);
	XFreeFontNames(fonts);

	// Create color information
	this->colors = new std::vector<XColor>();
	this->colormap = DefaultColormap(this->dpy, DefaultScreen(this->dpy));
	this->curColor = this->whiteColor;
	this->linecolors = new std::vector<unsigned long>();


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

// resets gui to draw again
void LogoGUI::reset()
{
	if(this->lines) delete this->lines;
	this->lines = new std::vector<int>();
	int i;
	for(i=0;i<this->colors->size();i++)
	{
		unsigned long pixel = this->colors->at(i).pixel;
		XFreeColors(this->dpy, this->colormap, &pixel, 1 , 0);
	}
	delete this->colors;
	delete this->linecolors;
	this->linecolors = new std::vector<unsigned long>();
	this->colors = new std::vector<XColor>();
	this->curColor = this->whiteColor;
	this->x = this->width / 2;
	this->y = (this->height - 100) / 2;
	this->pen = true;
	this->angle = 270;

	if(this->sayTxts) {
		for(i=0; i < this->sayTxts->size(); i++)
		{
			free(this->sayTxts->at(i).text);
		}
		delete this->sayTxts;
	}
	this->sayTxts = new std::vector<sayTxt>();
	XClearWindow(this->dpy, this->win);
	this->drawInterface(NULL);
	XFlush(this->dpy);
}

void LogoGUI::setColor(int r, int g, int b)
{
	r *= 256;
	g *= 256;
	b *= 256;
	int i;
	XColor c;
	for(i=0;i<this->colors->size();i++)
	{
		c = this->colors->at(i);
		if(c.red == r && c.green == g && c.blue == b)
		{
			this->curColor = c.pixel;
			return;
		}
	}	
	c.red = r;
	c.green = g;
	c.blue = b;
	XAllocColor(this->dpy, this->colormap, &c);
	this->colors->push_back(c);
	this->curColor = c.pixel;
}

// draw line to main window
void LogoGUI::drawLine(int x1, int y1, int x2, int y2)
{
	XDrawLine(this->dpy, this->win, this->gc, x1, y1, x2, y2);
}

// draw the interface at the bottom of the window
void LogoGUI::drawInterface(char *c)
{
	XSetForeground(this->dpy, this->gc, this->whiteColor);

	XClearArea(this->dpy, this->win, 0 , 400, 400, 100, false);

	this->drawLog();
	XClearArea(this->dpy, this->win, 370 , 455, 400, 485, false);

	// CMD history
	this->drawLine(0, 400, 400, 400);
	this->drawLine(0, 400, 0, 500);
	this->drawLine(400, 400, 400, 500);

	// CMD input
	this->drawLine(0, 485, 400, 485);
	this->drawLine(0, 499, 400, 499);

	// Run Button
	this->drawLine(370, 485, 370, 499);
	this->drawText(371, 497, "RUN");

	// CLEAR Button
	this->drawLine(370, 485, 370, 470);
	this->drawLine(370, 470, 400, 470);
	this->drawLine(400, 485, 400, 470);
	this->drawText(370, 482, "CLEAR");

	// STOP Button
	this->drawLine(370, 455, 370, 470);
	this->drawLine(370, 455, 400, 455);
	this->drawLine(400, 455, 400, 470);
	this->drawText(370, 468, "STOP");

	this->drawCursor(c);
	
	XSetForeground(this->dpy, this->gc, this->curColor);
}

// draw input cursor
// its a simple line with moves as text is given
void LogoGUI::drawCursor(char *c)
{
	int x = 10;
	if(c)
	{
		x += XTextWidth(this->fontstruct, c, strlen(c));
	}
	this->drawLine(x, 488, x, 497);
}

// add last command to history
void LogoGUI::logStr(const char *c)
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
	this->drawInterface(NULL);
	XFlush(this->dpy);
}

// draw the command history to screen
void LogoGUI::drawLog()
{
	int i;
	for(i=0;i<this->nextLog;i++)
	{
		this->drawText(10, 418 + i * 15, this->log[i]);
	}
}

void LogoGUI::clearLog()
{
	int i;
	for(i=0;i<this->nextLog;i++)
		free(this->log[i]);
	this->nextLog = 0;
}

// repaint everything
void LogoGUI::restore()
{
	int i;
	for(i=0; i < this->lines->size(); i+=4)
	{
		XSetForeground(this->dpy, this->gc, this->linecolors->at(i/4));
		XDrawLine(this->dpy, this->win, this->gc, this->lines->at(i), this->lines->at(i+1),
			       this->lines->at(i+2), this->lines->at(i+3));
	}
	for(i=0; i < this->sayTxts->size(); i++)
	{
		struct sayTxt txt = this->sayTxts->at(i);
		XSetForeground(this->dpy, this->gc, txt.color);
		this->drawText(txt.x, txt.y, txt.text);
	}
	XSetForeground(this->dpy, this->gc, this->curColor);


}

void LogoGUI::sayText(const char *text)
{
	this->drawText(this->x, this->y, text);
	struct sayTxt txt;
	txt.x = this->x;
	txt.y = this->y;
	txt.color = this->curColor;
	txt.text = (char*) malloc(strlen(text) + 1);
	strcpy(txt.text, text);
	this->sayTxts->push_back(txt);
}

bool LogoGUI::checkAbort()
{
	if(this->abort) return true;
	XEvent e;
	XButtonEvent *be = (XButtonEvent *) &e;
	while(XPending(this->dpy))
	{
		XNextEvent(this->dpy, &e);
		if(e.type == ButtonPress)
		{
			if(be->x > 370 && be->y < 470 && be->y > 455)
			{
				this->abort = true;
				return true;
			}
		}
	}
	return false;
}

void LogoGUI::clearAbort()
{
	this->abort = false;
}


// read string and process events
void LogoGUI::readString(char *buf, int len)
{ 
	
	buf[0] = 0;
	XEvent e;
	XKeyPressedEvent *ke = (XKeyPressedEvent*) &e;
	XButtonEvent *be = (XButtonEvent*) &e;
	char c;
	char *drawbuf;
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
		//draw the interface and the text
		drawbuf = buf;

		// Make the command fit.
		while(XTextWidth(this->fontstruct, drawbuf, strlen(drawbuf)) > 355 && *drawbuf)
		{
			drawbuf++;
		}
		this->drawInterface(drawbuf);
		XSetForeground(this->dpy, this->gc, this->whiteColor);
		this->drawText(10, 497, drawbuf);
		XSetForeground(this->dpy, this->gc, this->curColor);
		XFlush(this->dpy);
		XNextEvent(this->dpy, &e);

		if(e.type == KeyPress)
		{
			// Process input
		       	XmbLookupString(xic, ke, &c, 1, &ks, &s);
			if(ks==XK_Return) break;
			if(ks==XK_BackSpace) {
				if(i)
					buf[--i] = 0;
			} else {
				if(ks==XK_space) {
					c = ' ';
				}
				if(c)
				{
					buf[i] = c;
					buf[++i] = 0;
				}
			}
		} else if(e.type == ButtonPress) {
			// Process mouse click
			if(be->x > 370 && be->y > 485) // RUN has selected
			{
				break;
			} else if(be->x > 370 && be->y > 470 && be->y < 485) { // Clear
				this->reset();
			} else if(be->y < 493 && be->y > 405) { // a command in history
				int index = (be->y - 405) / 15;
				if(this->nextLog > index && strlen(this->log[index]) < len) {
					strcpy(buf, this->log[index]);
					i = strlen(buf);
				}
			}
		} else if(e.type == Expose) { // Restore the window
			this->restore();
		}
	}
	this->logStr(buf); // add a copy to the log
	XDestroyIC(xic);
	XCloseIM(xim);
}

// Draw a Text with the first 12px fixed font found
void LogoGUI::drawText(int x, int y, const char *text)
{
	XTextItem t[1];
	t->chars = (char*) text;
	t->nchars = strlen(text);
	t->font = this->font;
	t->delta = 1;
	XDrawText(this->dpy, this->win, this->gc, x, y, t, 1);
}

// Go to x, y
void LogoGUI::setxy(double x, double y) {
	this->x = x;
	this->y = y;
}

// Go forward
// Convert coordinate systems
void LogoGUI::fd(double len) {
	double ex, ey;
	double ang = this->angle * 0.0174533;
	ex = this->x + len * cos(ang);
	ey = this->y + len * sin(ang);
	if(this->pen) {
		this->linecolors->push_back(this->curColor);
	 	XSetForeground(this->dpy, this->gc, this->curColor);
		// Remember the lines
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

// backward
void LogoGUI::bd(double len) {
	this->fd(len*-1);	
}

// right
// TODO: numbers bigger than 360.
void LogoGUI::rt(double angle) {
	this->angle += angle;
	if(this->angle >= 360) this->angle -= 360;
}

// left
// TODO: numbers bigger than 360
void LogoGUI::lt(double angle) {
	this->angle -= angle;
	if(this->angle < 0) this->angle += 360;
}

// Pen up
void LogoGUI::pu() {
	this->pen = false;
}

// Pen down
void LogoGUI::pd() {
	this->pen = true;
}
