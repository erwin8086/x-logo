#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"Parser.h"

int main()
{
	const char* cmd = "repeat 8 [ repeat 4 [ fd 100 rt 90 ] rt 45 ]";
	LogoGUI *g = new LogoGUI(400, 500);
	Parser *p = new Parser(cmd);
	p->execute(g);
	delete p;
	char* next = (char*) malloc(256);
	while(1)
	{
		g->readString(next, 256);
		next[255] = 0;
		if(strcmp(next, "exit") == 0)
			break;
		Parser *p = new Parser(next);
		g->reset();
		p->execute(g);
		delete p;
	}
	free(next);
}
