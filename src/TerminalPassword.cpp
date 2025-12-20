#include "TerminalPassword.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#else
#include <termios.h>
#include <unistd.h>
#include <print>
#endif

void SetEcho(bool enable)
{
#ifdef _WIN32
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;
	GetConsoleMode(hStdin, &mode);

	if (!enable)
		mode &= ~ENABLE_ECHO_INPUT;
	else
		mode |= ENABLE_ECHO_INPUT;

	SetConsoleMode(hStdin, mode);

#else

	std::print("\n"); // hack

	struct termios tty;
	tcgetattr(STDIN_FILENO, &tty);

	if (!enable)
		tty.c_lflag &= ~ECHO;
	else
		tty.c_lflag |= ECHO;

	(void)tcsetattr(STDIN_FILENO, TCSANOW, &tty);

#endif
}