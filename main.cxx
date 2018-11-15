#include<unistd.h>
#include"GUI.h"

int main()
{
	LogoGUI *g = new LogoGUI(800, 800);
	g->bd(100);
	g->rt(90);
	g->bd(100);
	g->rt(90);
	g->bd(100);
	g->rt(90);
	g->bd(100);
	g->rt(90);
	sleep(10);
}
