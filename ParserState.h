#include<vector>

#ifndef _PARSERSTATE_H_
#define _PARSERSTATE_H_

#define PS_VARS std::vector<struct var>
#define PS_PROCS std::vector<struct proc>
#define PS_STRVARS std::vector<struct strVar>

struct var {
        char *name;
        double val;
};

struct proc {
	char *name;
	char *commands;
};

struct strVar {
	char *name;
	char *val;
};

class ParserState
{
	public:
		ParserState();
		ParserState(ParserState *parent);
		~ParserState();
		bool getDelay();
		void setDelay(bool delay);
		double getVar(const char *name);
		void setVar(const char *name, double val);
		bool isVar(const char *name);
		void listVars();
		bool isProc(const char *name);
		void setProc(const char *name, const char *cmds);
		const char* getProc(const char *name);
		void push(double d);
		double pop();
		std::vector<double>* getStack();
		void listProcs();
		bool isStrVar(const char *name);
		const char* getStrVar(const char *name);
		void setStrVar(const char *name, const char *value);
	private:
		PS_VARS *vars;
		bool delay;
		ParserState *parent;
		PS_PROCS *procs;
		std::vector<double> *stack;
		PS_STRVARS *strVars;	

};

#endif
