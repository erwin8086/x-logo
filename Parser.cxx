#include<stdlib.h>
#include<string.h>
#include<time.h>
#include"Parser.h"
#include<stdio.h>
#include<assert.h>
#include<math.h>
#include<unistd.h>
#define TK_UNKNOWN -1 
#define TK_END 0
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
#define TK_CLEARCONSOLE 19
#define TK_WHILE 20
#define TK_COLOR 21
#define TK_MAKESTR 22
#define TK_SAY 23

#define EXPECTSTR {while(this->isSpace(*this->text)) this->text++; \
	if(!this->expectString(this->text)) { this->outError("[ERR] String expected\n"); return; }}
#define EXPECTNUM {while(this->isSpace(*this->text)) this->text++; \
	if(!this->expectNumber(this->text)) { this->outError("[ERR] Number expected\n"); return; }}
#define EXPECTCMD {while(this->isSpace(*this->text)) this->text++; \
	if(!this->expectCmdList(this->text)) { this->outError("[ERR] CmdList expected\n"); return; }}

Parser::Parser(const char* text, ParserState *parserState, LogoGUI *gui)
{
	this->text = text;
	this->parserState = parserState;
	this->gui = gui;
}

// Execute text on a per command base
// this looks like
// fd -> number -> rt -> number
// or repeat -> number -> [cmds]
// it starts with a command
// if command requires a parameter it is interpreted
// no error checking anywhere here
void Parser::execute()
{
	LogoGUI *g = this->gui;
	int t = this->nextToken();
	int rep, repcount;
	int i, red, green, blue;
	char *name;
	double val;
	char *cmd, *fname, *buf;
	FILE *f;
	const char *proc;
	const char *oldtext, *newtext;
	while(t && !g->checkAbort())
	{
		// Execute the token in LogoGUI
		switch(t)
		{
			case TK_FD:
				EXPECTNUM;
				g->fd(this->nextNumber());
				break;
			case TK_BD:
				EXPECTNUM;
				g->bd(this->nextNumber());
				break;
			case TK_RT:
				EXPECTNUM;
				g->rt(this->nextNumber());
				break;
			case TK_LT:
				EXPECTNUM;
				g->lt(this->nextNumber());
				break;
			case TK_PU:
				g->pu();
				break;
			case TK_PD:
				g->pd();
				break;
			case TK_REPEAT:
				EXPECTNUM;
				rep = this->nextNumber();
				EXPECTCMD;
				cmd = this->nextCmdList();
				repcount = 1;
				while(rep-- > 0 && !g->checkAbort()){
					Parser *p = new Parser(cmd, this->parserState, g);
					p->repcount = repcount++;
					p->execute();
					delete p;
				}
				free(cmd);
				break;
			case TK_WHEN:
				EXPECTNUM;
				rep = this->nextNumber();
				EXPECTCMD;
				cmd = this->nextCmdList();
				if(rep > 0.001 || rep < -0.001)
				{
					Parser *p = new Parser(cmd, this->parserState, g);
					p->execute();
					delete p;
				}
				free(cmd);
				break;
			case TK_CLEAR:
				g->reset();
				break;
			case TK_MAKE:
				EXPECTSTR;
				name = this->nextString();
				EXPECTNUM;
				val = this->nextNumber();
				this->parserState->setVar(name, val);
				free(name);
				break;
			case TK_PRINT:
			case TK_SAY:
				if(this->isStrNext())
				{
					EXPECTSTR;
					cmd = this->nextString();
				} else {
					cmd = (char*) malloc(32);
					EXPECTNUM;
					strfromd(cmd, 32, "%f", this->nextNumber());
				}
				if(t==TK_PRINT)
					g->logStr(cmd);
				else
					g->sayText(cmd);
				free(cmd);
				break;
			case TK_FAST:
				this->parserState->setDelay(false);
				break;
			case TK_SLOW:
				this->parserState->setDelay(true);
				break;
			case TK_LOAD:
				EXPECTSTR;
				fname = this->nextString();
				f = fopen(fname, "r");
				if(!f)
				{
					buf = (char*) malloc(256);
					sprintf(buf, "[ERR] File not found: %s\n", fname);
					this->outError(buf);
					free(buf);
					free(fname);
					return;
				}	
				free(fname);
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
					Parser *p = new Parser(cmd, this->parserState,g);
					p->execute();
					delete p;
				}
				free(cmd);	
				break;
			case TK_TO:
				EXPECTSTR;
				name = this->nextString();
				EXPECTCMD;
				cmd = this->nextCmdList();
				this->parserState->setProc(name, cmd);
				break;
			case TK_PROC:
				proc = this->parserState->getProc(this->lastProc);
				if(proc)
				{
					ParserState *pS = new ParserState(this->parserState);
					Parser *p = new Parser(proc, pS, g);
					p->execute();
					delete p;
					delete pS;
				}
				break;	
			case TK_PUSH:
				EXPECTNUM;
				val = this->nextNumber();
				this->parserState->push(val);
				break;
			case TK_SLEEP:
				EXPECTNUM;
				val = this->nextNumber();
				sleep((int)val);
				break;
			case TK_UNKNOWN:
				buf = (char*) malloc(256);
				snprintf(buf, 256, "[ERR] Unknown token: %s\n", this->text);
				this->outError(buf);
				free(buf);
				return;
			case TK_CLEARCONSOLE:
				g->clearLog();
				break;
			case TK_WHILE:
				EXPECTNUM;
				oldtext = this->text;
				val = this->nextNumber();
				EXPECTCMD;
				cmd = this->nextCmdList();
				newtext = this->text;
				while(val && !g->checkAbort())
				{
					Parser *p = new Parser(cmd, this->parserState, g);
					p->execute();
					delete p;
					this->text = oldtext;
					val = this->nextNumber();
				}
				this->text = newtext;
				free(cmd);
				break;
			case TK_COLOR:
				EXPECTNUM;
				red = (int) this->nextNumber();
				EXPECTNUM;
				green = (int) this->nextNumber();
				EXPECTNUM;
				blue = (int) this->nextNumber();
				g->setColor(red, green, blue);
				break;	
			case TK_MAKESTR:
				EXPECTSTR;
				name = this->nextString();
				EXPECTSTR;
				buf = this->nextString();
				this->parserState->setStrVar(name, buf);
				free(buf);
				free(name);
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

void Parser::outError(const char *text)
{
	char *buf = (char*) malloc(strlen(text)+1);
	char *t = buf;
	strcpy(buf, text);
	while(*t)
	{
		if(*t == '\n') *t = '\0';
		if(*t == '\t') *t = ' ';
		t++;
	}
	this->gui->logStr(buf);
	free(buf);
}

#define STR_PARAM(x) { while(this->isSpace(**text)) (*text)++; \
	if(**text != '(' && **text != ',') x=NULL; \
	(*text)++; x=this->nextString(text); }
#define NUM_PARAM(x) { while(this->isSpace(**text)) (*text)++; \
	if(**text != '(' && **text != ',') x=0; \
	(*text)++; x=this->nextNumber(text); }
#define STR_REQ(x) { if(!x) { x = (char*) malloc(1); *x = 0; } }

char* Parser::getStringFunction(const char **text)
{
	if(**text != '$') return NULL;
	(*text)++;
	if(!**text || this->isSpace(**text)) return NULL;
	const char *start = *text;
	while(**text && !this->isSpace(**text) && **text != '(' && 
	      **text != ')' && **text != ',') (*text)++;
	char *buf = (char*) malloc(*text - start + 1);
	memcpy(buf, start, *text - start);
	buf[*text - start] = 0;
	if(**text != '(')
	{
		if(!this->parserState->isStrVar(buf))
		{
			char *msg = (char*) malloc(256);
			snprintf(msg, 256, "Unknown string variable: %s\n", buf);
			this->outError(msg);
			free(msg);
		}
		const char *var = this->parserState->getStrVar(buf);
		if(!var) var = "";
		free(buf);
		buf = (char*) malloc(strlen(var) + 1);
		strcpy(buf, var);
		return buf;
	}
	char *funcName = buf;
	char *res = NULL;
	if(strcmp(funcName, "var")==0)
	{
		STR_PARAM(buf);
		STR_REQ(buf);
		if(!this->parserState->isStrVar(buf))
		{
			char *msg = (char*) malloc(256);
			snprintf(msg, 256, "Unknown string variable: %s\n", buf);
			this->outError(msg);
			free(msg);
		}
		const char *var = this->parserState->getStrVar(buf);
		if(!var) var = "";
		res = (char*) malloc(strlen(var) + 1);
		strcpy(res, var);
		free(buf);
	}
	else if(strcmp(funcName, "cat")==0)
	{
		char *a, *b;
		STR_PARAM(a);
		STR_PARAM(b);
		STR_REQ(a);
		STR_REQ(b);
		res = (char*) malloc(strlen(a) + strlen(b) + 1);
		strcpy(res, a);
		strcat(res, b);
		free(a); free(b);
	}
	else if(strcmp(funcName, "tostring")==0)
	{
		double n;
		NUM_PARAM(n);
		res = (char*) malloc(32);
		strfromd(res, 32, "%f", n);
	}
	else if(strcmp(funcName, "int")==0)
	{
		double n;
		NUM_PARAM(n);
		res = (char*) malloc(32);
		snprintf(res, 32, "%i", (int) n);
	}
	while(this->isSpace(**text)) (*text)++;
	if(**text != ')')
	{
		this->outError("Missing ')' in string function");
	} else {
		(*text)++;
	}
	free(funcName);
	if(!res)
	{
		res = (char*) malloc(1);
		*res = 0;
	}
	return res;

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
	return this->nextString(&this->text);
}

char* Parser::nextString(const char **text)
{
	while(this->isSpace(**text)) (*text)++;
	char type = **text;
	if(type == '[') return this->nextCmdList(text);
	if(type == '$') return this->getStringFunction(text);
	if(type != '"') return NULL;
	(*text)++;
	const char *start = *text;
	while(**text && !this->isSpace(**text) &&
	      **text != ',' && **text != ')') (*text)++;
	const char *end = *text;
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
	if(**text == ':' || **text == '$')
	{
		(*text)++;
		while(**text && !this->isSpace(**text) 
				&& **text != '(' && **text != ')' && **text != ','
				&& **text != '+' && **text != '-' &&
				**text != '*' && **text != '/') (*text)++;
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
		if(!this->isSpace(*text))
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

#define EXPECTPAR(x) { if(this->numFuncParam(text) != x) { this->outError("To many or to less parameters\n"); return 0; } }
#define STR_PARAM(x) { while(this->isSpace(*text)) (text)++; \
	if(*text != '(' && *text != ',') x=NULL; \
	(text)++; x=this->nextString(&text); }
#define STR_REQ(x) { if(!x) { x = (char*) malloc(1); *x = 0; } }

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

	time_t timep = time(NULL);
	struct tm *t = localtime(&timep);

	double res = 0;
	if(strcmp(funcName, ":mod")==0)
	{
		EXPECTPAR(2);
		double a = this->getFuncParam(&text);
		double b = this->getFuncParam(&text);
		res = (int)a % (int)b;
	}
	else if(strcmp(funcName, ":sin")==0)
	{
		EXPECTPAR(1);
		double a = this->getFuncParam(&text);
		a *= 0.0174533;
		res = sin(a);
	}
	else if(strcmp(funcName, ":cos")==0)
	{
		EXPECTPAR(1);
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
		EXPECTPAR(2);
		res = this->getFuncParam(&text) > this->getFuncParam(&text);
	}
	else if(strcmp(funcName, ":lt")==0)
	{
		EXPECTPAR(2);
		res = this->getFuncParam(&text) < this->getFuncParam(&text);
	}
	else if(strcmp(funcName, ":equ")==0)
	{
		EXPECTPAR(2);
		res = this->getFuncParam(&text) == this->getFuncParam(&text);
	}
	else if(strcmp(funcName, ":not")==0)
	{
		EXPECTPAR(1);
		res = ! this->getFuncParam(&text);
	}
	else if(strcmp(funcName, ":int")==0)
	{
		EXPECTPAR(1);
		res = (int) this->getFuncParam(&text);
	}
	else if(strcmp(funcName, ":pop")==0)
	{
		EXPECTPAR(0);
		res = this->parserState->pop();
	}	
	else if(strcmp(funcName, ":year")==0)
	{
		EXPECTPAR(0);
		res = t->tm_year + 1900;
	}
	else if(strcmp(funcName, ":mon")==0)
	{
		EXPECTPAR(0);
		res = t->tm_mon + 1;
	}
	else if(strcmp(funcName, ":day")==0)
	{
		if(params == 0)
		{
			res = t->tm_mday;
		} else {
			int sel = (int) this->getFuncParam(&text);
			switch(sel)
			{
				case 0:
					res = t->tm_wday;
					break;
				case 1:
					res = t->tm_mday;
					break;
				case 2:
					res = t->tm_yday;
					break;
			}
		}
	}
	else if(strcmp(funcName, ":hour")==0)
	{
		EXPECTPAR(0);
		res = t->tm_hour;
	}
	else if(strcmp(funcName, ":min")==0)
	{
		EXPECTPAR(0);
		res = t->tm_min;
	}
	else if(strcmp(funcName, ":sec")==0)
	{
		EXPECTPAR(0);
		res = t->tm_sec;
	}
	else if(strcmp(funcName, ":var")==0)
	{
		EXPECTPAR(1);
		char *var;
		STR_PARAM(var);
		STR_REQ(var);
		if(!this->parserState->isVar(var))
		{
			char *msg = (char*) malloc(256);
			snprintf(msg, 256, "Unknown variable: %s\n", var);
			this->outError(msg);
			free(msg);
			res = 0;
		} else {
			res = this->parserState->getVar(var);
		}
		free(var);
	}
	else if(strcmp(funcName, ":tonumber")==0)
	{
		EXPECTPAR(1);
		char *num;
		STR_PARAM(num);
		STR_REQ(num);
		res = atof(num);
		free(num);

	}
	free(funcName);
	return res;
}

double Parser::nextNumber(const char **text)
{
	int l;
	double r = this->nextNumber(*text, &l);
	*text += l;
	return r;
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
	while(this->isSpace(*text)) text++;
	start = text;
	bool wasSpace = false;
	bool wasOp = false;
	while(*text >= '0' && *text <= '9' || *text == '.' ||
			*text == '+' || *text == '-' || *text == '*' ||
			*text == '/' || this->isSpace(*text) || *text == ':') 
	{
		if(this->isSpace(*text))
		{
			wasSpace = true;
		}
		else if(*text == '+' || *text == '-' ||
				*text == '*' || *text == '/') 
		{
			wasSpace=false;
			wasOp = true;
		}
		else if(((*text >=0 && *text <= '9') ||
					*text == ':') && wasOp)
		{
			wasSpace = false;
			wasOp = false;
		}
		else if(wasSpace && ((*text >=0 && *text <= '9') ||
					*text == ':'))
		{
			break;
		}
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

							if(this->isSpace(left[i]))
							{
								left[i] = 0;
							}
						}

						if(strcmp(var, "repcount")==0)
						{
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

char* Parser::nextCmdList()
{
	return this->nextCmdList(&this->text);
}

// Interpret next thing as a list of commands
// surrounded by [ ]
// works with nested repeats
char* Parser::nextCmdList(const char **text)
{
	int opens = -1;
	const char *start;
	while(**text && opens != 0)
	{
		if(**text == '[')
		{
			if(opens==-1)
			{
				start = *text;
				opens = 1;
			}
			else
			{
				opens++;
			}
		} else if(**text == ']') {
			opens--;
		}
		(*text)++;
	}
	// create a substring to return
	char *sub = (char*) malloc(*text - start - 1);
	memcpy(sub, start + 1, *text - start - 2);
	sub[*text - start - 2] = 0;
	return sub;
}	

bool Parser::isStrNext()
{
	int i=0;
	while(this->text[i] && this->isSpace(this->text[i]) ) i++;
	if(this->text[i] == '"' || this->text[i] == '[' || this->text[i] == '$') return true;
	return false;
}

bool Parser::expectToken(const char *text, const char *expect)
{
	while(*expect)
	{
		if(*text != *expect || (!*text))
			return false;
		text++; expect++;
	}
	return true;
}

bool Parser::isSpace(char c)
{
	if(c == ' ' || c == '\t' || c == '\n')
		return true;
	return false;
}

bool Parser::expectCmdList(const char *text)
{
	if(*text != '[')
		return false;
	int depth = 1;
	text++;
	while(*text && depth)
	{
		if(*text == '[') depth++;
		if(*text == ']') depth--;
		text++;
	}
	if(depth == 0) return true;
	return false;
}

bool Parser::expectString(const char *text)
{
	if(*text != '"' && *text != '[' && *text != '$')
		return false;
	if( (*text == '"' || *text == '$') && *++text)
		return true;
	if(*text == '[')
	{
		text++;
		int depth = 1;
		while(depth && *text)
		{
			if(*text == '[') depth++;
			if(*text == ']') depth--;
			text++;
		}
		if(depth == 0)
			return true;
	}
	return false;
}

bool Parser::expectNumber(const char *text)
{
	if( (*text >= '0' && *text <= '9') || *text == '.' || *text == '-')
		if((*text == '.' || *text == '-') && *++text >= '0' && *text <= '9')
			return true;
		else if(*text >= '0' && *text <= '9')
			return true;
	if(*text == ':' && *++text && !this->isSpace(*text))
		return true;
	return false;
}

struct cmd {
	const char *cmd;
	int token;
};

struct cmd cmds[] = {
	{"push", TK_PUSH},
	{"pu", TK_PU},
	{"pd", TK_PD},
	{"print", TK_PRINT},
	{"fd", TK_FD},
	{"fast", TK_FAST},
	{"bd", TK_BD},
	{"rt", TK_RT},
	{"repeat", TK_REPEAT},	
	{"lt", TK_LT},
	{"load", TK_LOAD},
	{"clearconsole", TK_CLEARCONSOLE}, // Must before clear to work
	{"clear", TK_CLEAR},
	{"make$", TK_MAKESTR}, // Must before make to work
	{"make", TK_MAKE},
	{"when", TK_WHEN},
	{"sleep", TK_SLEEP},
	{"slow", TK_SLOW},
	{"to", TK_TO},
	{"while", TK_WHILE},
	{"color", TK_COLOR},
	{"say", TK_SAY},
	{NULL, 0}
};

// Interpret next thing as a command
// no error checking
// reaaaa == repeat
int Parser::nextToken()
{
	while(this->isSpace(*this->text) ) this->text++;

	// check if it is a procedure
	int i=0;
	while(this->text[i] && !this->isSpace(this->text[i])) i++;
	this->lastProc = (char*) malloc(i + 1);
	memcpy(this->lastProc, this->text, i);
	this->lastProc[i] = 0;
	if(this->parserState->isProc(this->lastProc))
	{
		this->text += i;
		return TK_PROC;
	}
	free(this->lastProc);
	this->lastProc = NULL;

	if(! *text)
		return TK_END;

	for(i=0; cmds[i].cmd; i++)
	{
		if(this->expectToken(text, cmds[i].cmd))
		{
			text += strlen(cmds[i].cmd);
			return cmds[i].token;
		}
	}

	return TK_UNKNOWN;
}
