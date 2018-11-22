#include<X11/Xlib.h>
#include<vector>

#ifndef _GUI_H_
#define _GUI_H_

class LogoGUI {
	public:
		LogoGUI(int width, int height);
		void setxy(double x, double y);
		void fd(double len);
		void bd(double len);
		void rt(double angle);
		void lt(double angle);
		void pu();
		void pd();
		void reset();
		void readString(char *buf, int len);
		void logStr(const char *c);
		bool checkAbort();
		void clearAbort();
		void clearLog();
	private:
		double x, y;
		int width, height;
		double angle;
		bool pen;
		Display *dpy;
		Window win;
		GC gc;
		int blackColor;
		int whiteColor;
		void drawInterface(char *c);
		void drawLine(int x1, int y1, int x2, int y2);
		void drawCursor(char *txt);
		Font font;
		XFontStruct *fontstruct;
		void drawText(int x, int y, const char *text);
		char *log[5] = {NULL, NULL, NULL, NULL, NULL};
		int nextLog = 0;
		void drawLog();
		std::vector<int> *lines;
		void restore();
		bool abort = false;

};
#endif
