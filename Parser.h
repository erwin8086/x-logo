#include"GUI.h"
#include<vector>
struct var {
	char *name;
	double val;
};

class Parser {
	public:
		Parser(const char* cmd, std::vector<struct var> *vars);
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
		std::vector<struct var> *vars;
};
