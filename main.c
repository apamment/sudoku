#if WIN32
#   define _MSC_VER 1
#	define _CRT_SECURE_NO_WARNINGS
#   include <windows.h>
#else
#
#endif

#include <OpenDoor.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

int solution[9][9];
int board[9][9];
int guess[9][9];

struct highscore_t {
	char name[64];
	time_t score;
}__attribute__((packed));


struct highscore_t scores[10];

void load_highscores() {
	FILE *fptr;
	int i;
	
	for (i=0;i<10;i++) {
		sprintf(scores[i].name, "No-One");
		scores[i].score = 59 * 60 + 59;
	}
	
	fptr = fopen("scores.dat", "rb");
	if (fptr) {
		for (i=0;i<10;i++) {
			if (fread(&scores[i], sizeof(struct highscore_t), 1, fptr) == 0) {
				break;
			}
		}
		fclose(fptr);
	}
}

void update_highscores(time_t score) {
	int added = 0;
	int i;
	int j;
	for (i=0;i<10;i++) {
		if (score < scores[i].score) {
			added = 1;
			for (j=9;j>i;j--) {
				sprintf(scores[j].name, "%s", scores[j-1].name);
				scores[j].score = scores[j-1].score;
			}
			scores[i].score = score;
			snprintf(scores[i].name, 63, "%s", od_control_get()->user_name);
			break;
		}
	}
	
	if (added) {
		FILE *fptr;
		
		fptr = fopen("scores.dat", "wb");
		if (fptr) {
			for (i=0;i<10;i++) {
				fwrite(&scores[i], sizeof(struct highscore_t), 1, fptr);
			}
			fclose(fptr);
		}
	}
}

void print_high_scores() {
	int i;
	for (i=0;i<10;i++) {
		od_set_cursor(5 + i, 39);
		od_printf("%s", scores[i].name);
		od_set_cursor(5 + i, 65);
		od_printf("%02d:%02d:%02d", scores[i].score / 60 / 60, scores[i].score / 60 % 60, scores[i].score % 60);
	}
}

void generate() {
	int k = 1;
	int n = 1;
	int i;
	int j;

	for (i = 0; i<9; i++) {
		k = n;
		for (j = 0; j<9; j++) {
			if (k <= 9) {
				solution[i][j] = k;
				k++;
			}
			else {
				k = 1;
				solution[i][j] = k;
				k++;
			}
		}
		n = k + 3;
		if (k == 10)
			n = 4;
		if (n>9)
			n = (n % 9) + 1;
   }
}

void permutation_row(int k1, int k2) {
	int temp;
	int j;
	for (j = 0; j<9; j++) {
		temp = solution[k1][j];
		solution[k1][j] = solution[k2][j];
		solution[k2][j] = temp;
	}
}

void permutation_col(int k1, int k2) {
	int temp;
	int j;
	for (j = 0; j<9; j++) {
		temp = solution[j][k1];
		solution[j][k1] = solution[j][k2];
		solution[j][k2] = temp;
	}
}

void row_change(int k1, int k2) {
	int temp;
	int n;
	int i;
	for (n = 1; n <= 3; n++) {
		for (i = 0; i<9; i++) {
			temp = solution[k1][i];
			solution[k1][i] = solution[k2][i];
			solution[k2][i] = temp;
		}
		k1++;
		k2++;
	}
}

void col_change(int k1, int k2) {
	int temp;
	int n;
	int i;
	for (n = 1; n <= 3; n++) {
		for (i = 0; i<9; i++) {
			temp = solution[i][k1];
			solution[i][k1] = solution[i][k2];
			solution[i][k2] = temp;
		}
		k1++;
		k2++;
	}
}

void random_gen(int check) {
	int k1;
	int k2;
	int max = 2;
	int min = 0;
	int i;
	for (i = 0; i<3; i++) {
		k1 = rand() % (max - min + 1) + min;
		do {
			k2 = rand() % (max - min + 1) + min;
		} while (k1 == k2);
		max += 3; min += 3;

		if (check == 1)
			permutation_row(k1, k2);
		else if (check == 0)
			permutation_col(k1, k2);
	}
}

void strike_out(int k1, int k2) {
	int row_from;
	int row_to;
	int col_from;
	int col_to;
	int i, j, b, c;
	int rem1, rem2;
	int flag;
	int temp = board[k1][k2];
	int count = 9;
	for (i = 1; i <= 9; i++) {
		flag = 1;
		for (j = 0; j<9; j++) {
			if (j != k2)
			{
				if (i != board[k1][j]) {
					continue;
				}
				else {
					flag = 0;
					break;
				}
			}
		}
		if (flag == 1) {
			for (c = 0; c<9; c++) {
				if (c != k1) {
					if (i != board[c][k2]) {
						continue;
					}
					else {
						flag = 0;
						break;
					}
				}
			}
		}
		if (flag == 1) {
			rem1 = k1 % 3; rem2 = k2 % 3;
			row_from = k1 - rem1;
			row_to = k1 + (2 - rem1);
			col_from = k2 - rem2;
			col_to = k2 + (2 - rem2);
			for (c = row_from; c <= row_to; c++) {
				for (b = col_from; b <= col_to; b++) {
					if (c != k1 && b != k2) {
						if (i != board[c][b])
							continue;
						else {
							flag = 0;
							break;
						}
					}
				}
			}
		}
		if (flag == 0)
			count--;
	}
	if (count == 1) {
		board[k1][k2] = 0;
	}
}

void new_puzzle() {
	int counter = 1, k1, k2;
	generate();
	random_gen(1);
	random_gen(0);

	int n[] = { 0,3,6 };
	for (int i = 0; i < 2; i++)
	{
		k1 = n[rand() % 3];
		do {
			k2 = n[rand() % 3];
		} while (k1 == k2);
		if (counter == 1)
			row_change(k1, k2);
		else col_change(k1, k2);
		counter++;
	}

	int max = 8;
	int min = 0;

	for (k1 = 0; k1 < 9; k1++) {
		for (k2 = 0; k2 < 9; k2++) {
			board[k1][k2] = solution[k1][k2];
		}
	}

	for (k1 = 0; k1<9; k1++) {
		for (k2 = 0; k2<9; k2++)
			strike_out(k1, k2);
	}
}

int check_solution() {
	int k;
	int j;
	for (j = 0; j < 9; j++) {
		for (k = 0; k < 9; k++) {
			if (guess[j][k] != solution[j][k]) {
				return 0;
			}
		}
	}
	return 1;
}


void update_timer(time_t starttime) {
	time_t timenow;
	time_t timediff;
	int hours = 0;
	int minutes = 0;
	int seconds = 0;

	timenow = time(NULL);

	timediff = timenow - starttime;

	hours = timediff / 60 / 60;
	minutes = timediff / 60 % 60;
	seconds = timediff % 60;

	od_set_cursor(14, 53);
	od_printf("`bright white`%02d:%02d:%02d", hours, minutes, seconds);
}


#if _MSC_VER
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
#else
int main(int argc, char **argv)
{
#endif
	int done = 0;
	int game_over = 0;
	int i;
	int j;
	int row_off = 0;
	int col_off = 0;
	int pos_x = 0;
	int pos_y = 0;
	time_t starttime;
	tODInputEvent ch;
#if _MSC_VER
	od_parse_cmd_line(lpszCmdLine);
#else
	od_parse_cmd_line(argc, argv);
#endif
	od_init();



	srand(time(NULL));


	load_highscores();


	while (!done) {

		od_clr_scr();
		od_send_file("suscore.ans");

		print_high_scores();

		od_get_input(&ch, OD_NO_TIMEOUT, GETIN_NORMAL);
		if (ch.EventType == EVENT_CHARACTER) {
			if (tolower(ch.chKeyPress) == 'q') {
				done = 1;
				break;
			} else if (tolower(ch.chKeyPress) == 'n') {
				od_clr_scr();
				od_send_file("sudoku.ans");
				new_puzzle();
				for (i = 0; i < 9; i++) {
					row_off = i / 3;
					for (j = 0; j < 9; j++) {
						col_off = j / 3;
						od_set_cursor(5 + i + row_off, 9 + j * 3 + col_off);
						guess[i][j] = board[i][j];
						if (board[i][j] == 0) {
							od_printf("`white` ");
						}
						else {
							od_printf("`white`%d", board[i][j]);
						}
					}
				}

				row_off = pos_y / 3;
				col_off = pos_x / 3;
				od_set_cursor(5 + pos_y + row_off, 9 + pos_x * 3 + col_off);

				game_over = 0;

				starttime = time(NULL);

				while (!game_over) {
					if (od_get_input(&ch, 1000, GETIN_NORMAL)) {
						if (ch.EventType == EVENT_EXTENDED_KEY) {
							do {
								switch (ch.chKeyPress) {
								case OD_KEY_DOWN: //down
									if (pos_y == 8) {
										pos_y = 0;
									}
									else {
										pos_y++;
									}
									break;
								case OD_KEY_UP: //up
									if (pos_y == 0) {
										pos_y = 8;
									}
									else {
										pos_y--;
									}
									break;
								case OD_KEY_LEFT: //left
									if (pos_x == 0) {
										pos_x = 8;
									}
									else {
										pos_x--;
									}
									break;
								case OD_KEY_RIGHT: //right
									if (pos_x == 8) {
										pos_x = 0;
									}
									else {
										pos_x++;
									}
									break;
								}
							} while (board[pos_y][pos_x] != 0);
						}
						else if (ch.EventType == EVENT_CHARACTER) {
							switch (ch.chKeyPress) {
							case 'q':
							case 'Q':
								game_over = 1;
								break;
							case '1':
							case '2':
							case '3':
							case '4':
							case '5':
							case '6':
							case '7':
							case '8':
							case '9':
								guess[pos_y][pos_x] = ch.chKeyPress - '0';
								od_printf("`bright yellow`%d", ch.chKeyPress - '0');
								row_off = pos_y / 3;
								col_off = pos_x / 3;
								od_set_cursor(5 + pos_y + row_off, 9 + pos_x * 3 + col_off);
								break;
							case 'c':
							case 'C':
								if (check_solution()) {
									od_set_cursor(18, 21);
									od_printf("`bright green`Congratulations! You Won! Press Any Key!");
									od_get_key(TRUE);
									od_set_cursor(18, 21);
									od_printf("                                        ");
									update_highscores(time(NULL) - starttime);
									game_over = 1;
								}
								else {
									od_set_cursor(18, 21);
									od_printf("`bright red`Sorry, solution is not correct! Keep Trying.");
								}
								break;
							}
						}
					}

					update_timer(starttime);

					row_off	= pos_y / 3;
					col_off = pos_x / 3;
					od_set_cursor(5 + pos_y + row_off, 9 + pos_x * 3 + col_off);
				}
			}
		}
	}
	od_exit(0, FALSE);
}
