#include"GUI.h"
class Parser {
	public:
		Parser(const char* cmd);
		void execute(LogoGUI *gui);
	private:
		const char *text;
		int nextToken();	
		double nextNumber();
		char* nextCmdList();
};
