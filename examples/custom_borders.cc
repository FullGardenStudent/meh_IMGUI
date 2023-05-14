#define MEH_USE_RENDERER
#include "meh_IMGUI/meh_IMGUI.hh"

// define custom top and left hit functions
// by default left, right and bottom will have a width of 5 pixels and top title bar will be 22 pixels long
// you can disable border at specific regions if needed(my game engine definitely needs it).
bool top(uint32_t mousex, uint32_t mousey, long top, long left, long right, long bottom) {
	// exclude 250 pixels on top left and 100 pixels on top right from title bar.
	return (mousey >= top && mousey < top + 22 &&
		mousex >= left + 250 && mousex <= right - 100);
}

// make left border more wider than default.
bool left(uint32_t mousex, uint32_t mousey, long top, long left, long right, long bottom) {
	// return (mousex > left && mousex < left + 20);
	if (mousex > left && mousex < left + 20) {
		std::cout << "left hit !!!" << std::endl;
		return true;
	}
	else {
		return false;
	}
}

int main() {

	if (!meh::create_window("something", 50, 50, 500, 500, MEH_BORDERLESS_WINDOW)) {
		std::cout << "Failed to create window!" << std::endl;
	}

	// pass custom hit functions
	meh::set_top_hit(top);
	meh::set_left_hit(left);
	
	while (true) {
		if (!meh::update_window()) {
			break;
		}
	}

	return meh::meh_cleanup();
}