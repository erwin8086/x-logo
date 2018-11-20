#include"GUI.h"
#include<vector>
struct var {
	char *name;
	double val;
};

class Parser {
	public:
		Parser(const char* cmd, std::vector<struct var> *vars, bool delay);
		bool getSpeed();
		void execute(LogoGUI *gui);
		int repcount;
	private:
		const char *text;
		int nextToken();	
		double nextNumber();
		double nextNumber(const char *text);
		double nextNumber(const char *text, int *len);
		char* nextCmdList();
		char* nextString();
		bool isStrNext();
		void skipFunc(const char **text);
		double getFunc(const char *text);
		double getFuncParam(const char **text);
		int numFuncParam(const char *text);
		std::vector<struct var> *vars;
		bool delay;
};
