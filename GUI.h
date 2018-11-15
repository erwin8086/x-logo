#include<X11/Xlib.h>
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

};
