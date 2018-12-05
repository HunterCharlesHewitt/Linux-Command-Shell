#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/param.h>

using namespace std;

vector<int> background_vect;
vector<string> back_name;
char temp[MAXPATHLEN];
string cds = get_current_dir_name();
struct Command
{
	string cmd = "";
	vector<string> arg_vect;
	bool has_in_arg = false;
	bool has_out_arg = false;
};

class Shell
{
private:
	string cmd_str = " "; //string that is the total user input 
	vector<string> token_vect; //vector which contains all tokens separated by spaces, except for those contained in quotation marks, or those following echo and not followed by quotation marks
	vector<Command> cmd_vect;
	bool has_pipe = false; //information on whether or not the command has a pipe not in quotes
	bool has_in = false; //information on whether or not the command has an input redirect not in quotes
	bool has_out = false; //information on whether or not the command has an output redirection not in quotes
	bool has_and = false;
	bool has_jobs = false;
	bool has_cd = false;
	int pid; //will be used to create a child process with fork when necessary 
public:
	void debug();
	void prompt_user();
	void tokenize();
	void make_cmd_vect();
	void parse_pipe();
	void run_jobs();
	void run_cd();
	string handle_redirect(vector<string> &v);
	void run_basic(Command c);
	void run_pipe(Command cmd, bool last, int fd[], int& a);
	void run_background(Command cmds);
	void check_background();
	void set_out(string file, const char** c);
	void set_in(string file, const char** c);
	const char** make_arg_cvect(Command d);
};

