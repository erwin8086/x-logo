#include<vector>

#ifndef _PARSERSTATE_H_
#define _PARSERSTATE_H_

#define PS_VARS std::vector<struct var>
#define PS_PROCS std::vector<struct proc>

struct var {
        char *name;
        double val;
};

struct proc {
	char *name;
	char *commands;
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
	private:
		PS_VARS *vars;
		bool delay;
		ParserState *parent;
		PS_PROCS *procs;
		std::vector<double> *stack;
};

#endif
