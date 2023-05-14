#include "meh_IMGUI/meh_IMGUI.hh"

int main() {

	if (!meh::create_window("something", 50, 50, 500, 500, MEH_NORMAL_WINDOW)) {
		std::cout << "Failed to create window!" << std::endl;
	}

	while (true) {
		if (!meh::update_window()) {
			break;
		}
	}

	return meh::meh_cleanup();
}