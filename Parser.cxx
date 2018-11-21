#include<stdlib.h>
#include<string.h>
#include<time.h>
#include"Parser.h"
#include<stdio.h>
#include<assert.h>
#include<math.h>
#include<unistd.h>
#define TK_FD 1
#define TK_BD 2
#define TK_RT 3
#define TK_LT 4
#define TK_PU 5
#define TK_PD 6
#define TK_REPEAT 7
#define TK_CLEAR 8
#define TK_MAKE 9
#define TK_PRINT 10
#define TK_WHEN 11
#define TK_SLOW 12
#define TK_FAST 13
#define TK_LOAD 14
#define TK_TO 15
#define TK_PROC 16
#define TK_PUSH 17
#define TK_SLEEP 18

Parser::Parser(const char* text, ParserState *parserState)
{
	this->text = text;
	this->parserState = parserState;
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
	char *name;
	double val;
	char *cmd, *fname, *buf;
	FILE *f;
	const char *proc;
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
					Parser *p = new Parser(cmd, this->parserState);
					p->repcount = repcount++;
					p->execute(g);
					delete p;
				}
				free(cmd);
				break;
			case TK_WHEN:
				rep = this->nextNumber();
				cmd = this->nextCmdList();
				if(rep > 0.001 || rep < -0.001)
				{
					Parser *p = new Parser(cmd, this->parserState);
					p->execute(g);
					delete p;
				}
				free(cmd);
				break;
			case TK_CLEAR:
				g->reset();
				break;
			case TK_MAKE:
				name = this->nextString();
				val = this->nextNumber();
				this->parserState->setVar(name, val);
				free(name);
				break;
			case TK_PRINT:
				if(this->isStrNext())
				{
					cmd = this->nextString();
					g->logStr(cmd);
					free(cmd);
				} else {
					cmd = (char*) malloc(32);
					strfromd(cmd, 32, "%f", this->nextNumber());
					g->logStr(cmd);
					free(cmd);
				}
				break;
			case TK_FAST:
				this->parserState->setDelay(false);
				break;
			case TK_SLOW:
				this->parserState->setDelay(true);
				break;
			case TK_LOAD:
				fname = this->nextString();
				f = fopen(fname, "r");
				free(fname);
				assert(f);
				buf = (char*) malloc(256);
				cmd = NULL;
				while(!feof(f))
				{
					size_t len = fread(buf, 1, 255, f);
					buf[len] = 0;
					if(cmd)
					{
						char *ncmd = (char*) malloc(strlen(buf) + strlen(cmd) + 1);
						strcpy(ncmd, cmd);
						strcat(ncmd, buf);
						free(cmd);
						cmd = ncmd;
					} else {
						cmd = buf;
						buf = (char*) malloc(256);
					}
				}		
				fclose(f);
				free(buf);
				{
					Parser *p = new Parser(cmd, this->parserState);
					p->execute(g);
					delete p;
				}
				free(cmd);	
				break;
			case TK_TO:
				name = this->nextString();
				cmd = this->nextCmdList();
				this->parserState->setProc(name, cmd);
				break;
			case TK_PROC:
				proc = this->parserState->getProc(this->lastProc);
				if(proc)
				{
					ParserState *pS = new ParserState(this->parserState);
					Parser *p = new Parser(proc, pS);
					p->execute(g);
					delete p;
					delete pS;
				}
				break;	
			case TK_PUSH:
				val = this->nextNumber();
				this->parserState->push(val);
				break;
			case TK_SLEEP:
				double val = this->nextNumber();
				sleep((int)val);
				break;
		}
		// Wait 10ms and read next Command
		if(this->parserState->getDelay())
		{
			struct timespec ti;
			ti.tv_sec = 0;
			ti.tv_nsec = 10000000;
			while(nanosleep(&ti, &ti));
		}
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
	if(*this->text == '[') return this->nextCmdList();
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

void Parser::skipFunc(const char **text)
{
	int depth;
	if(**text == ':')
	{
		(*text)++;
		while(**text && **text != ' ' && **text != '\t' && **text !='\n'
		      && **text != '(' && **text != ')' && **text != ',') (*text)++;
		if(**text == '(')
		{
			(*text)++;
			depth = 1;
			while(**text && depth > 0)
			{
				if(**text == ')') depth--;
				if(**text == '(') depth++;
				(*text)++;
			}
			assert(depth==0);
		}
		(*text)--;
	}
}

double Parser::getFuncParam(const char **text)
{
	(*text)++;
	const char *start = *text;
	while(**text && **text != ')' && **text != ',')
	{
		this->skipFunc(text);
		printf("skipFunc: %s\n", *text);
		(*text)++;
	}	
	assert(**text);
	char *param = (char*)malloc(*text - start + 1);
	memcpy(param, start, *text - start);
	param[*text - start] = 0;
	double res = this->nextNumber(param);
	free(param);
	return res;

}

int Parser::numFuncParam(const char *text)
{
	int params = 0;
	text++;
	while(*text && *text != ')')
	{
		if(*text != ' ' && *text != '\t' && *text != '\n')
		{
			params++;
			while(*text && *text != ',' && *text != ')')
			{
				this->skipFunc(&text);
				text++;
			}
		}
		text++;
	}
	return params;
}

double Parser::getFunc(const char *text)
{
	const char *start, *end;
	char *funcName;
	start = text;
	while(*text && *text != '(') text++;
	assert(*text == '(');
	end = text;
	funcName = (char*) malloc(end - start + 1);
	memcpy(funcName, start, end - start);
	funcName[end - start] = 0;
	
	int params = this->numFuncParam(text);

	double res = 0;
	if(strcmp(funcName, ":mod")==0)
	{
		double a = this->getFuncParam(&text);
		double b = this->getFuncParam(&text);
		res = (int)a % (int)b;
	}
	else if(strcmp(funcName, ":sin")==0)
	{
		double a = this->getFuncParam(&text);
		a *= 0.0174533;
		res = sin(a);
	}
	else if(strcmp(funcName, ":cos")==0)
	{
		double a = this->getFuncParam(&text);
		a *= 0.0174533;
		res = cos(a);
	}
	else if(strcmp(funcName, ":random")==0)
	{
		res = (double)rand() / (double)RAND_MAX;
		if(params == 1)
		{
			res *= this->getFuncParam(&text);
		} else if(params == 2) {
			double a = this->getFuncParam(&text);
			double b = this->getFuncParam(&text);
			res *= (b - a);
			res += a;
		}
	}
	else if(strcmp(funcName, ":gt")==0)
	{
		res = this->getFuncParam(&text) > this->getFuncParam(&text);
	}
	else if(strcmp(funcName, ":lt")==0)
	{
		res = this->getFuncParam(&text) < this->getFuncParam(&text);
	}
	else if(strcmp(funcName, ":equ")==0)
	{
		res = this->getFuncParam(&text) == this->getFuncParam(&text);
	}
	else if(strcmp(funcName, ":int")==0)
	{
		res = (int) this->getFuncParam(&text);
	}
	else if(strcmp(funcName, ":pop")==0)
	{
		res = this->parserState->pop();
	}	
	free(funcName);
	return res;
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
	int i=0, depth;
	while(*text == ' ' || *text == '\n' || *text == '\t') text++;
	start = text;
	while(*text >= '0' && *text <= '9' || *text == '.' ||
              *text == '+' || *text == '-' || *text == '*' ||
	      *text == '/' || *text == ' ' || *text == '\t' || *text == ':') 
	{
		this->skipFunc(&text);
		text++;
	}
	end = text - 1;
	if(len)
		*len = text - startText;
	for(op=start; op <= end; op++)
	{
		this->skipFunc(&op);
		if(*op=='+') break;
	}
	if(*op != '+')
	{
		for(op=start; op <= end; op++)
		{
			this->skipFunc(&op);
			if(*op=='-') break;
		}
		if(*op != '-')
		{
			for(op=start; op <= end; op++)
			{
				this->skipFunc(&op);
				if(*op=='*') break;
			}
			if(*op!='*')
			{
				for(op=start; op <= end; op++)
				{
					this->skipFunc(&op);
					if(*op=='/') break;
				}
				if(*op!='/')
				{
					left = (char*) malloc(end - start + 2);
					memcpy(left, start, end - start + 1);
					left[end - start + 1] = 0;
					if(*left == ':')
					{
						char *var = left + 1;
						for(i=0; i<(end-start+1); i++)
						{
							if(left[i] == '(') // a function
							{
								res = this->getFunc(left);
								free(left);
								return res;
							}

							if(left[i] == ' ' || left[i] == '\t' || 
							   left[i] == '\n')
							{
								left[i] = 0;
							}
						}

						if(strcmp(var, "repcount")==0)
						{
							printf("repcount");
							res = this->repcount;
						} else {
							res = this->parserState->getVar(var);
						}
					} else {
						res = atof(left);
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

bool Parser::isStrNext()
{
	int i=0;
	while(this->text[i] && (this->text[i] == ' ' ||
	      this->text[i] == '\t' || this->text[i] == '\n') ) i++;
	printf("isStrNext: %c\n", this->text[i]);
	if(this->text[i] == '"' || this->text[i] == '[') return true;
	return false;
}
	
// Interpret next thing as a command
// no error checking
// reaaaa == repeat
int Parser::nextToken()
{
	while(*this->text == ' ' || *this->text == '\n') this->text++;

	// check if it is a procedure
	int i=0;
	while(this->text[i] && this->text[i] != ' ' && 
	      this->text[i] != '\n' && this->text[i] != '\t')
		i++;
	this->lastProc = (char*) malloc(i);
	memcpy(this->lastProc, this->text, i);
	this->lastProc[i] = 0;
	printf("lastProc = %s\n", this->lastProc);
	if(this->parserState->isProc(this->lastProc))
	{
		this->text += i;
		return TK_PROC;
	}
	free(this->lastProc);
	this->lastProc = NULL;

	char a = *this->text; char b = *(++this->text); this->text++;
	switch(a)
	{
		case 'p':
			if(b == 'u' && *this->text == 's')
			{
				this->text += 2;
				return TK_PUSH;
			}
			if(b == 'u')
			{
				return TK_PU;
			}
			else if(b=='r')
			{
				this->text += 3;
				return TK_PRINT;
			} else {
				return TK_PD;
			}
		case 'f':
			if(b=='d')
				return TK_FD;
			this->text += 2;
			return TK_FAST;

		case 'b':
			return TK_BD;
		case 'r':
			if(b == 't') 
				return TK_RT;
			this->text += 4;
			return TK_REPEAT;
		case 'l':
			if(b=='t')
				return TK_LT;
			this->text += 2;
			return TK_LOAD;
		case 'c':
			this->text += 3;
			return TK_CLEAR;
		case 'm':
			this->text += 2;
			return TK_MAKE;
		case 'w':
			this->text += 2;
			return TK_WHEN;
		case 's':
			if(*this->text == 'e')
			{
				this->text += 3;
				return TK_SLEEP;
			}
			this->text += 2;
			return TK_SLOW;
		case 't':
			return TK_TO;

	}
	return 0;
}
