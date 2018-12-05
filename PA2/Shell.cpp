#include "Shell.h"

void Shell::debug()
{
	
	/*for (int i = 0; i < token_vect.size(); ++i)
	{
		cout << token_vect[i] << endl;
	}*/
	for (int i = 0; i < cmd_vect.size(); ++i)
	{
		cout << cmd_vect[i].cmd << endl;
		for (int j = 0; j < cmd_vect[i].arg_vect.size(); ++j)
		{
			cout  << "********************" << cmd_vect[i].arg_vect[j] << endl;
		}
	}
}

void Shell::prompt_user()
{
	Command cmd1;
	cout << "Linux Command Line:$~ ";
	getline(cin, cmd_str);
	if(cmd_str.find_first_not_of(' ') == string::npos)
	{
		return;
	}
	tokenize();
	make_cmd_vect();
	//debug();
	if(has_pipe == false && cmd_str.size() != 0)
	{
		cmd1 = cmd_vect[0];
		run_basic(cmd1);
	}
	else if(has_pipe == true && cmd_str.size() != 0)
	{
		parse_pipe();
	}
	check_background();
}

void Shell::tokenize()
{
	string sub = "";
	bool in_d_quotes = false;
	bool in_s_quotes = false;
	bool this_one = false;
	for(int i = 0; i < cmd_str.length(); ++i)
	{
		if(cmd_str[i] == '|' && in_d_quotes == false && in_s_quotes == false)
		{
			has_pipe = true;
		}
		if(cmd_str[i] == '<' && in_d_quotes == false && in_s_quotes == false)
		{
			has_in = true;
		}
		if(cmd_str[i] == '>' && in_d_quotes == false && in_s_quotes == false)
		{
			has_out = true;
		}
		if(cmd_str[i] == '&' && in_d_quotes == false && in_s_quotes == false)
		{
			has_and = true;
		}		
		if(i < cmd_str.size() - 3 && cmd_str[i] == 'j' && cmd_str[i+1] == 'o' && cmd_str[i+2] == 'b' && cmd_str[i+3] == 's' && in_d_quotes == false && in_s_quotes == false)
		{
			has_jobs = true;
		}	
		if(i < cmd_str.size() -1 && cmd_str[i] == 'c' && cmd_str[i+1] == 'd' && in_d_quotes == false && in_s_quotes == false)
		{
			has_cd = true;
		}

		if(cmd_str[i] == '"' && in_s_quotes == false)
		{

			in_d_quotes = !in_d_quotes;
			this_one = true;
		}
		if(cmd_str[i] == '\'' && in_d_quotes == false)
		{
			in_s_quotes = !in_s_quotes;
			this_one = true;
		}

		//first check to see if the command contains a pipe, file input, or file output. This will be useful for later
		if((cmd_str[i] != ' ' && cmd_str[i] != '"' && cmd_str[i] != '\'') || (in_s_quotes == true || in_d_quotes == true) && !this_one)
		{
			sub += cmd_str[i];
			if(i == cmd_str.length()-1)
			{
				token_vect.push_back(sub);
				sub = "";
				return;
			}
		}
		else
		{
			if(sub != "")
			{
				token_vect.push_back(sub);
			}
			sub = "";
		}
		this_one = false;
	}
}

void Shell::make_cmd_vect()
{
	bool in_once = false;
	int arg_pos = 0;
	Command dummy_cmd;
	if(token_vect.size() > 0)
	{
		dummy_cmd.cmd = token_vect[0];
	}

	for(int i = 1; i < token_vect.size(); ++i)
	{
		if((has_in) && token_vect[i].find("<") != string::npos)
		{
			dummy_cmd.has_in_arg = true;
		}
		if((has_out) && token_vect[i].find(">") != string::npos)
		{
			dummy_cmd.has_out_arg = true;
		}
		if(token_vect[i].find('|') != string::npos) //REWRITE
		{
			cmd_vect.push_back(dummy_cmd);
			arg_pos = 0;
			dummy_cmd.has_in_arg = false;
			dummy_cmd.has_out_arg = false;
			dummy_cmd.arg_vect = {};
			dummy_cmd.cmd = "|";
			cmd_vect.push_back(dummy_cmd);
			if(token_vect[i].length() > 1)
			{
				dummy_cmd.cmd = token_vect[i].substr(2, token_vect[1].length()-1);
			}
			else if(i < token_vect.size()-1)
			{
				dummy_cmd.cmd = token_vect[i+1];
			}
			in_once = true;
		}
		else
		{
			if(!in_once)
			{
				arg_pos ++;
				dummy_cmd.arg_vect.push_back(token_vect[i]);
			}
			in_once = false;
		}

	}
	if(token_vect.size() > 0)
	{
		cmd_vect.push_back(dummy_cmd);
	}
}

void Shell::run_basic(Command cmdt)
{
	int i = 0;
	const char **argv;
	string dummy = "";
	vector<string> new_vect;
	if((cmdt.has_out_arg))
	{
		dummy = handle_redirect(cmdt.arg_vect);
		argv = make_arg_cvect(cmdt);
		argv[0] = cmdt.cmd.c_str();
		set_out(dummy, argv);
		return;
	}
	else if((cmdt.has_in_arg))
	{
		dummy = handle_redirect(cmdt.arg_vect);
		argv = make_arg_cvect(cmdt);
		argv[0] = cmdt.cmd.c_str();
		set_in(dummy, argv);
		return;
	}

	if(has_and)
	{
		run_background(cmdt);
		return;
	}

	if(has_jobs)
	{
		run_jobs();
		return;
	}

	if(has_cd)
	{
		run_cd();
		return;
	}

	if(cmdt.arg_vect.size() > 0)
	{
		argv = make_arg_cvect(cmdt);
		argv[0] = cmdt.cmd.c_str();
		pid = fork();
		if(pid == 0)
		{
			execvp(argv[0], (char**)argv);
		}
		else
		{
			wait(&pid);
		}
	}
	else
	{
		pid = fork();
		if(pid == 0)
		{
			execlp(cmdt.cmd.c_str(), cmdt.cmd.c_str(), NULL);
		}
		else
			wait(&pid);	
	}
}

void Shell::run_background(Command cmdt)
{
	const char **argv;
	cmdt.arg_vect.resize(cmdt.arg_vect.size()-1);
	argv = make_arg_cvect(cmdt);
	argv[0] = cmdt.cmd.c_str();
	int pids = fork();
	if(pids == 0)
	{
		execvp(argv[0], (char**)argv);
	}
	else
	{
		background_vect.push_back(pids);
		back_name.push_back(cmdt.cmd);
	}
}

void Shell:: check_background()
{
	int wpid;
	int status;
	for(int i =0; i < background_vect.size(); ++i)
	{
		wpid = waitpid(background_vect[i], &status, WNOHANG);
		if(wpid == 0)
		{
		}
		else
		{
			wait(&background_vect[i]);
			background_vect.erase(background_vect.begin() + i);
			back_name.erase(back_name.begin() + i);
		}
	}
}

void Shell:: run_jobs()
{
	for(int i = 0; i < background_vect.size(); i++)
	{
		cout << "[" << i << "]" << '\t' << "Running: " << '\t'<< '\t' << '\t'  << back_name[i] << endl;
	}
}
void Shell::parse_pipe()
{
	bool last = false;
	vector<Command> pipe_cmds;
	for(int i = 0; i < cmd_vect.size(); ++i)
	{
		if(cmd_vect[i].cmd.find("|") == string::npos)
		{
			pipe_cmds.push_back(cmd_vect[i]);
		}
	}
	int save_std_in = dup(0);
	int save_std_out = dup(1);
	int fd[2];
	int next_in = 0;
	for(int i = 0; i < pipe_cmds.size(); ++i)
	{
		if(i == pipe_cmds.size()-1)
		{
			last = true;
		}
		run_pipe(pipe_cmds[i], last, fd, next_in);
	}
	
	dup2(save_std_in, 0);
	dup2(save_std_out, 1);
}
void Shell::run_pipe(Command cmd, bool last, int fd[], int& next_in)
{
	bool in = false;
	bool out = false;
	int x;
	const char **argv;
	argv = make_arg_cvect(cmd);
	argv[0] = cmd.cmd.c_str();
	string dummy = "";
	pipe(fd);
	if(fork() == 0)
	{
		if((cmd.has_out_arg))
		{
			out = true;
			dummy = handle_redirect(cmd.arg_vect);
			argv = make_arg_cvect(cmd);
			argv[0] = cmd.cmd.c_str();
			x = open (dummy.c_str(), O_CREAT|O_WRONLY, S_IRUSR | S_IWUSR);
			dup2(next_in, 0);
			dup2(x, 1);
			close(fd[0]);
			execvp(argv[0], (char**)argv);
		}
		else if((cmd.has_in_arg))
		{
			in = true;
			dummy = handle_redirect(cmd.arg_vect);
			argv = make_arg_cvect(cmd);
			argv[0] = cmd.cmd.c_str();
			x = open (dummy.c_str(), O_CREAT| O_RDONLY , S_IREAD | S_IWRITE);
			dup2(x, 0);
			if(!last)
			{
				dup2(fd[1], 1);
			}
			close(fd[0]);
			execvp(argv[0], (char**)argv);
		}
		else
		{
			dup2(next_in, 0);
			if(!last)
			{
				dup2(fd[1], 1);
			}
			close(fd[0]);
			execvp(argv[0], (char**)argv);
		}
	}
	else
	{
		wait(0);
		close(fd[1]);
		next_in = fd[0];
	}
}

void Shell:: run_cd()
{
	string subs = "";
	for (int i = 0; i < cmd_str.size(); ++i)
	{
		if(i < cmd_str.size() - 1 && cmd_str[i] == 'c' && cmd_str[i+1] == 'd')
		{
			++i;
		}
		else if(cmd_str[i] != ' ')
		{
			subs += cmd_str[i];
		}
	}
	if (subs.find("-") != string::npos)
	{
		chdir(cds.c_str());
	}
	else
	{
		cds = get_current_dir_name();
		chdir(subs.c_str());
	}
}

string Shell::handle_redirect(vector<string> &v)
{
	int in_out = 0; // 1 is in, 2 is out
	int flex = 0;
	string file = "";

	for(int i = 0; i < v.size(); ++i)
	{
		for(int j = 0; j < v[i].size(); ++ j)
		{
			if(v[i].at(j) == '>')
			{
				
				in_out = 2;
				++j;
				flex = j;
				for(j; j < v[i].size(); ++j)
				{
					file += v[i].at(j);
				}
				v[i] = v[i].substr(0, flex-1);
				break;
			}
			else if(v[i].at(j) == '<')
			{
				in_out = 1;
				++j;
				flex = j;
				for(j; j < v[i].size(); ++j)
				{
					file += v[i].at(j);
				}
				v[i] = v[i].substr(0, flex-1);
				break;
			}
		}
		if(v[i] == "")
		{
			v.erase(v.begin() + i);
		}

		if(in_out > 0 && file == "")
		{
			file = v[i];
			v.erase(v.begin() + i);
			break;
		}
	}
	return file;
}	

void Shell::set_out(string file, const char** argv)
{
	int fd;
	pid = fork();
	if(pid == 0)
	{
		fd = open (file.c_str(), O_CREAT|O_WRONLY, S_IRUSR | S_IWUSR);
		dup2 (fd, 1);
		execvp(argv[0], (char**)argv);
	}
	else
		wait(&pid);	
}

void Shell::set_in(string file, const char** argv)
{
	int fd;
	pid = fork();
	if(pid == 0)
	{
		fd = open (file.c_str(), O_CREAT| O_RDONLY , S_IREAD | S_IWRITE);
		dup2 (fd, 0);
		execvp(argv[0], (char**)argv);
	}
	else
		wait(&pid);	
}

const char**  Shell:: make_arg_cvect(Command cmds)
{

	 const char **argv = new const char* [cmds.arg_vect.size()+2];
	 for(int i = 0; i < cmds.arg_vect.size(); ++i)
	 {
	 	argv[i+1] = cmds.arg_vect[i].c_str();
	 }
	 argv[cmds.arg_vect.size()+1] = NULL;
	 return argv; 
}


