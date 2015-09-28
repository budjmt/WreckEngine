#pragma once

struct Mouse {
	double x = 0, y = 0, prevx = 0, prevy = 0;
	bool down = 0;
	double clickCoolDown = 0.2, lastClick = 0;

	Mouse() {}
};