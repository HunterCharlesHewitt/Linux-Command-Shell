#include "Shell.cpp"

int main()
{
	while(true)
	{
		Shell *shell = new Shell();
		shell->prompt_user();
		delete shell;
	}
}