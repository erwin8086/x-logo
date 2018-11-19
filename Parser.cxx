#include<stdlib.h>
#include<string.h>
#include<time.h>
#include"Parser.h"
#include<stdio.h>
#define TK_FD 1
#define TK_BD 2
#define TK_RT 3
#define TK_LT 4
#define TK_PU 5
#define TK_PD 6
#define TK_REPEAT 7
#define TK_CLEAR 8
#define TK_MAKE 9

Parser::Parser(const char* text, std::vector<struct var> *vars)
{
	this->text = text;
	this->vars = vars;
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
	int rep, repcount;
	int i;
	struct var var;
	char *cmd;
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
				rep = this->nextNumber();
				cmd = this->nextCmdList();
				repcount = 1;
				while(rep-- > 0){
					Parser *p = new Parser(cmd, this->vars);
					p->repcount = repcount++;
					p->execute(g);
					delete p;
				}
				free(cmd);
				break;
			case TK_CLEAR:
				g->reset();
				break;
			case TK_MAKE:
				var.name = this->nextString();
				var.val = this->nextNumber();
				for(i=0; i<this->vars->size(); i++)
				{
					if(strcmp(this->vars->at(i).name, var.name)==0)
					{
						this->vars->erase(this->vars->cbegin()+i);
						break;
					}
				}
				this->vars->push_back(var);
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

double Parser::nextNumber()
{
	int len;
	double r = this->nextNumber(this->text, &len);
	this->text += len;
	return r;
}

char* Parser::nextString()
{
	while(*this->text == ' ' || *this->text == '\t') this->text++;
	if(*this->text != '"') return NULL;
	const char *start = ++this->text;
	while(*this->text && *this->text != ' ' &&
       	      *this->text != '\t' && *this->text != '\n') this->text++;
	const char *end = this->text;
	char *str = (char*) malloc(end - start + 1);
	memcpy(str, start, end - start);
	str[end - start] = 0;
	return str;
}

double Parser::nextNumber(const char *text)
{
	return this->nextNumber(text, (int*)NULL);
}

// Interpret next thing as a number
// Once again no error checking
double Parser::nextNumber(const char *text, int *len)
{
	const char *startText = text;
	char num[30];
	const char *start;
	const char *end;
	const char *op;
	char *left, *right;
	double res;
	int i=0;
	while(*text == ' ' || *text == '\n' || *text == '\t') text++;
	start = text;
	while(*text >= '0' && *text <= '9' || *text == '.' ||
              *text == '+' || *text == '-' || *text == '*' ||
	      *text == '/' || *text == ' ' || *text == '\t' || *text == ':') 
	{
		if(*text == ':')
		{
			text++;
			while(*text >= 'a' && *text <= 'z') text++;
		} else {	
			text++;
		}
	}
	end = text - 1;
	if(len)
		*len = text - startText;
	for(op=start; op <= end; op++)
	{
		if(*op=='+') break;
	}
	if(*op != '+')
	{
		for(op=start; op <= end; op++)
		{
			if(*op=='-') break;
		}
		if(*op != '-')
		{
			for(op=start; op <= end; op++)
			{
				if(*op=='*') break;
			}
			if(*op!='*')
			{
				for(op=start; op <= end; op++)
				{
					if(*op=='/') break;
				}
				if(*op!='/')
				{
					left = (char*) malloc(end - start + 2);
					memcpy(left, start, end - start + 1);
					left[end - start + 1] = 0;
					if(*left == ':')
					{
						printf("Colon: %s", left);
						char *var = left + 1;
						for(i=0; i<(end-start+1); i++)
						{
							if(left[i] == ' ' || left[i] == '\t')
							{
								left[i] = 0;
							}
						}
						if(strcmp(var, "repcount")==0)
						{
							printf("repcount");
							res = this->repcount;
						} else {
							for(i=0;i<this->vars->size(); i++)
							{
								if(strcmp(var, this->vars->at(i).name)==0)
								{
									res = this->vars->at(i).val;
								}
							}
						}
					} else {
						res = atol(left);
					}
					free(left);
					return res;

				}
			}
		}
	}
	left  = (char*) malloc(op - start + 1);
	right = (char*) malloc(end - op + 1);

	memcpy(left, start, op - start);
	memcpy(right, op + 1, end - op);

	left[op - start] = 0;
	right[end - op] = 0;

	printf("Op: %c, left: %s, right: %s", *op, left, right);

	switch(*op)
	{
		case '+':
			res = this->nextNumber(left) + this->nextNumber(right);
			break;
		case '-':
			res = this->nextNumber(left) - this->nextNumber(right);
			break;
		case '*':
			res = this->nextNumber(left) * this->nextNumber(right);
			break;
		case '/':
			res = this->nextNumber(left) / this->nextNumber(right);
			break;
	}
	free(left);
	free(right);
	return res;
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
		case 'c':
			this->text += 3;
			return TK_CLEAR;
		case 'm':
			this->text += 2;
			return TK_MAKE;
	}
	return 0;
}
