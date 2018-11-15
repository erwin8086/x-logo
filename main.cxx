#include<unistd.h>
#include"Parser.h"

int main()
{
	LogoGUI *g = new LogoGUI(800, 800);
	Parser *p = new Parser("repeat 8 [ repeat 4 [ fd 100 rt 90 ] rt 45 ]");
	p->execute(g);
	sleep(10);
}
