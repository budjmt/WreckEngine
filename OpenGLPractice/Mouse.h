#pragma once

struct Mouse {
	int button = 0;
	double x = 0, y = 0, prevx = 0, prevy = 0;
	bool down = false;
	double clickCoolDown = 0.2, lastClick = 0;
};