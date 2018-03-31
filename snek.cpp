#define SCREEN_WIDTH 35
#define SCREEN_HEIGHT 18
#define SNAKE_SIZE 20
#define MAX_MOVES 200

#include <ctime>
#include <chrono>
#include <cstdlib>
#include <deque>
#include <ncurses.h>
#include <unistd.h>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <thread>
#include <mutex>
#include <vector>

struct Bodypart {
public:
	bool operator==(const Bodypart &bp) const {
    	return (this->h == bp.h && this->w == bp.w);
    }

	int h;
	int w;
};

class Snake {
public:
	bool check_if_valid(const Bodypart new_head)
	{
		bool result = false;
		auto e = std::find(body.begin(), body.end(), new_head);
		if (e == body.end()) {
			result = true;
		}
		return result;
	}

	Snake()
	{
		for (int w = 1; w < SNAKE_SIZE; w++) {
			body.push_front({ 1, w });
		}

		head = { 1, SNAKE_SIZE - 1 };
	}

	std::deque<Bodypart> body;

	Bodypart head;
	static std::mutex mtx;
	bool is_alive { true };

	void move()
	{
		bool width_not_too_big { (bool)(head.w + 1 < SCREEN_WIDTH - 1) };
		bool width_not_too_small { (bool)(head.w - 1 >= 1) };
		bool height_not_too_big { (bool)(head.h + 1 < SCREEN_HEIGHT - 1) };
		bool height_not_too_small { (bool)(head.h - 1 >= 1) };

		bool already_moved = false;
		bool checked_possibilites[4] { false, false, false, false };

		while(!already_moved && is_alive)
		{
			int direction { rand() % 4 };
			switch(direction)
			{
				case 0:
				checked_possibilites[0] = true;
				if (width_not_too_big && check_if_valid({ head.h, head.w + 1 } )) {
					head.w++;
					body.push_front({ head.h, head.w });
					body.pop_back();
					already_moved = true;
					for (int i = 0; i < 4; i++) checked_possibilites[i] = false;
				}
				break;

				case 1:
				checked_possibilites[1] = true;
				if (width_not_too_small && check_if_valid({ head.h, head.w - 1 })) {
					head.w--;
					body.push_front({ head.h, head.w });
					body.pop_back();
					already_moved = true;
					for (int i = 0; i < 4; i++) checked_possibilites[i] = false;
				}
				break;

				case 2:
				checked_possibilites[2] = true;
				if (height_not_too_big && check_if_valid({ head.h + 1, head.w })) {
					head.h++;
					body.push_front({ head.h, head.w });
					body.pop_back();
					already_moved = true;
					for (int i = 0; i < 4; i++) checked_possibilites[i] = false;
				}
				break;

				case 3:
				checked_possibilites[3] = true;
				if (height_not_too_small && check_if_valid({ head.h - 1, head.w })) {
					head.h--;
					body.push_front({ head.h, head.w });
					body.pop_back();
					already_moved = true;
					for (int i = 0; i < 4; i++) checked_possibilites[i] = false;
				}
				break;
			}
		if (checked_possibilites[0] &&
			checked_possibilites[1] &&
			checked_possibilites[2] &&
			checked_possibilites[3] &&
			!already_moved) {
				is_alive = false;
			}
		}
	}

};

std::mutex Snake::mtx;

void start_snake(Snake snake, WINDOW* w)
{
	int moves_made { 0 };
	while(snake.is_alive && (moves_made < MAX_MOVES)) {
		// erase();
		Snake::mtx.lock();
		snake.move();
		
		werase(w);
		box(w, 0, 0);
		for (auto it = snake.body.begin(); it < snake.body.end(); it++) {
			wmove(w, it->h, it->w);
			waddch(w,  (char)219  /* '*' */ );
		}
		//
		wrefresh(w);
		moves_made++;
		Snake::mtx.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	Snake::mtx.lock();
	mvwprintw(w, SCREEN_WIDTH/4, SCREEN_HEIGHT/3, "* SNAKE IS DEAD *");
	wrefresh(w);
	Snake::mtx.unlock();
}

int main()
{
	srand(time(NULL));
	initscr();
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_RED);
	init_pair(2, COLOR_WHITE, COLOR_GREEN);
	init_pair(3, COLOR_WHITE, COLOR_BLUE);
	init_pair(4, COLOR_WHITE, COLOR_YELLOW);
	init_pair(5, COLOR_BLACK, COLOR_BLACK);
	color_set(5, NULL);
	curs_set(0);

	Snake snake1;
	Snake snake2;
	Snake snake3;
	Snake snake4;

	WINDOW* w1 = newwin(SCREEN_HEIGHT, SCREEN_WIDTH, 0, 0);
	WINDOW* w2 = newwin(SCREEN_HEIGHT, SCREEN_WIDTH, 0, SCREEN_WIDTH);
	WINDOW* w3 = newwin(SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	WINDOW* w4 = newwin(SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH);

	wbkgd(w1, COLOR_PAIR(1));
	wbkgd(w2, COLOR_PAIR(2));
	wbkgd(w3, COLOR_PAIR(3));
	wbkgd(w4, COLOR_PAIR(4));

	std::vector<std::thread> threads;

	threads.push_back(std::thread { start_snake, snake1, w1 });
	threads.push_back(std::thread { start_snake, snake2, w2 });
	threads.push_back(std::thread { start_snake, snake3, w3 });
	threads.push_back(std::thread { start_snake, snake4, w4 });

	for (auto& thread : threads) {
		thread.join();
	}

	endwin();
}
