#include "../meh_IMGUI.hh"

int main() {

	meh::MEH_WINDOW_INFO info = {
		"something", 50,50,500,500,MEH_NORMAL_WINDOW
	};

	if (!meh::create_window(info)) {
		std::cout << "Failed to create window!" << std::endl;
	}

	while (true) {
		if (!meh::update_window()) {
			break;
		}
	}

	meh::meh_cleanup();
	return 0;
}