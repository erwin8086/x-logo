#include<X11/Xlib.h>
class LogoGUI {
	public:
		LogoGUI(int width, int height);
		void setxy(int x, int y);
		void fd(int len);
		void bd(int len);
		void rt(int angle);
		void lt(int angle);
		void pu();
		void pd();
	private:
		int x, y;
		int width, height;
		int angle;
		bool pen;
		Display *dpy;
		Window win;
		GC gc;
		int blackColor;
		int whiteColor;

};
