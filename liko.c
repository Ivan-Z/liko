#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

// Key Mappings
#define CTRL_KEY(k) ((k) & 0x1f)

// Terminal control 
void die(const char* s) {
	// Clear entire screen
	write(STDOUT_FILENO, "\x1b[2J", 4);
	// Move cursor to top left
	write(STDOUT_FILENO, "\x1b[H", 3);

	perror(s);
	exit(1);
}

void disable_raw_mode() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
		die("tcsetattr");
}

void enable_raw_mode() {
	if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
		die("tcgetattr");
	atexit(disable_raw_mode);

	struct termios raw = orig_termios;	

	// Input flags:
	// IXON: Disable XON and XOFF signals (Ctrl-S and Ctrl-Q)
	// ICRNL: Disable turning carriage returns into new lines
	// BRKINT: Disable SIGINT on break condititons?
	// INPCK: Disable parity checking
	// ISTRIP: Disable bit stripping
	// IXON: Disable start/stop output control
	raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP | IXON);

	// Output flags:
	// OPOST: disable output post processing
	raw.c_oflag &= ~(OPOST);

	// Control flags:
	// CS8: Set 8 bit characters
	raw.c_cflag |= ~(CS8);

	// Local flags:
	// ECHO: disable text from being printed back to the screen
	// ICANON: disable cannonical mode (input being read in one line at a time)
	// ISIG: disable SIGINT, SIGSTP signals
	// IEXTEN: disable Ctrl-V sending literal characters instead of control characters
	// 			and disable Ctrl-O from discarding control characters
	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);

	// Control characters:
	// VMIN: maximum number of bytes needed before read returns
	// VTIME: maximum amount of time in 1/10s to wait before read returns
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;


	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
		die("tcsetattr");
}

char editor_read_key() {
  char c;
  int nread = read(STDIN_FILENO, &c, 1);
  if (nread == -1 && errno != EAGAIN)
	  die("read");
  return c;
}

void editor_process_keypress() {
	char c = editor_read_key();

	switch (c) {
		case CTRL_KEY('q'): {
			// Clear entire screen
			write(STDOUT_FILENO, "\x1b[2J", 4);
			// Move cursor to top left
			write(STDOUT_FILENO, "\x1b[H", 3);

			exit(0);
			break;
		}
	};
}

void editor_draw_rows() {
	for (int y = 0; y < 24; y++) {
		write(STDOUT_FILENO, "~\r\n", 3);
	}
}

void editor_refresh_screen() {
	// Clear entire screen
	write(STDOUT_FILENO, "\x1b[2J", 4);
	// Move cursor to top left
	write(STDOUT_FILENO, "\x1b[H", 3);

	editor_draw_rows();

	// Move cursor to top left
	write(STDOUT_FILENO, "\x1b[H", 3);
}

// Main
int main() {
	enable_raw_mode();

	while (1) {
		editor_refresh_screen();
		editor_process_keypress();
	}

	return 0;
}
