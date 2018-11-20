#include<unistd.h>
#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"Parser.h"

int main()
{
	std::vector<struct var> *vars = new std::vector<struct var>();
	// the exampe program
	const char* cmd = "repeat 8 [ repeat 4 [ fd 100 rt 90 ] rt 45 ] print [repeat 20 [ make \"a :repcount * 5 + 20 repeat 8 [ repeat 4 [ fd :a rt 90 ] rt 45 ] ] ] print [fast repeat 10000 [ fd :random(-10, 10) rt 90 * :int( :random(5) ) ] slow] print [make \"a :int( :random(3, 16) ) make \"b :int( :random(3, 16) ) repeat :a [ repeat :b [ fd 400 / :b rt 360 / :b ] rt 360 / :a ] ] print [fast repeat 80 [ make \"i :repcount repeat 35 [ fd :i rt 10 ] ] slow ] print [repeat 36 [ make \"len :repcount * 4 repeat 2 [ fd :len rt 90 fd :len / 2 rt 90 ] rt 10 ] ]";
	LogoGUI *g = new LogoGUI(400, 500);
	Parser *p = new Parser(cmd, vars, true);
	p->execute(g);
	delete p;

	srand(time(NULL));
	bool speed = true;

	// read the user program and execute
	char* next = (char*) malloc(256);
	while(1)
	{
		g->readString(next, 256);
		next[255] = 0;
		if(strcmp(next, "exit") == 0) // exit if "exit"
			break;
		Parser *p = new Parser(next, vars, speed);
		p->execute(g);
		speed = p->getSpeed();
		delete p;
	}
	free(next);
}
