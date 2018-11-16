#include<stdlib.h>
#include<string.h>
#include<time.h>
#include"Parser.h"
#define TK_FD 1
#define TK_BD 2
#define TK_RT 3
#define TK_LT 4
#define TK_PU 5
#define TK_PD 6
#define TK_REPEAT 7

Parser::Parser(const char* text)
{
	this->text = text;
}

// Execute text on a per command base
// this looks like
// fd -> number -> rt -> number
// or repeat -> number -> [cmds]
// it starts with a command
// if command requires a parameter it is interpreted
// no error checking anywhere here
void Parser::execute(LogoGUI *g)
{
	int t = this->nextToken();
	while(t)
	{
		// Execute the token in LogoGUI
		switch(t)
		{
			case TK_FD:
				g->fd(this->nextNumber());
				break;
			case TK_BD:
				g->bd(this->nextNumber());
				break;
			case TK_RT:
				g->rt(this->nextNumber());
				break;
			case TK_LT:
				g->lt(this->nextNumber());
				break;
			case TK_PU:
				g->pu();
				break;
			case TK_PD:
				g->pd();
				break;
			case TK_REPEAT:
				int rep = this->nextNumber();
				char *cmd = this->nextCmdList();
				while(rep-- > 0){
					Parser *p = new Parser(cmd);
					p->execute(g);
					delete p;
				}
				free(cmd);
				break;
		}
		// Wait 10ms and read next Command
		struct timespec ti;
		ti.tv_sec = 0;
		ti.tv_nsec = 10000000;
		while(nanosleep(&ti, &ti));
		t = this->nextToken();
	}
}

// Interpret next thing as a number
// Once again no error checking
double Parser::nextNumber()
{
	char num[30];
	int i=0;
	while(*this->text == ' ' || *this->text == '\n') this->text++;
	while( ((*this->text >= '0' && *this->text <= '9') || *this->text == '.' ) && i < 29) {
		num[i] = *this->text;
		i++;
		this->text++;
	}
	if(i>0)
	{
		num[i] = 0;
		return atol(num);

	} else {
		// if its not a number return 0
		return 0;
	}
}

// Interpret next thing as a list of commands
// surrounded by [ ]
// works with nested repeats
char* Parser::nextCmdList()
{
	int opens = -1;
	const char *start;
	while(*this->text && opens != 0)
	{
		if(*this->text == '[')
		{
			if(opens==-1)
			{
				start = this->text;
				opens = 1;
			}
			else
			{
				opens++;
			}
		} else if(*this->text == ']') {
			opens--;
		}
		this->text++;
	}
	// create a substring to return
	char *sub = (char*) malloc(this->text - start - 1);
	memcpy(sub, start + 1, this->text - start - 2);
	sub[this->text - start - 2] = 0;
	return sub;
}	
	
// Interpret next thing as a command
// no error checking
// reaaaa == repeat
int Parser::nextToken()
{
	while(*this->text == ' ' || *this->text == '\n') this->text++;
	char a = *this->text; char b = *(++this->text); this->text++;
	switch(a)
	{
		case 'p':
			if(b == 'u')
				return TK_PU;
			return TK_PD;
		case 'f':
			return TK_FD;
		case 'b':
			return TK_BD;
		case 'r':
			if(b == 't') 
				return TK_RT;
			this->text += 4;
			return TK_REPEAT;
		case 'l':
			return TK_LT;
	}
	return 0;
}
