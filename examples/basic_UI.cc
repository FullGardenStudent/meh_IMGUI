#define MEH_USE_RENDERER
#include "meh_IMGUI/meh_IMGUI.hh"
#include "meh_IMGUI/meh_IMGUI_widgets.hh"

int main() {

	if (!meh::create_window("something", 50, 50, 800, 900, MEH_BORDERLESS_WINDOW)) {
		std::cout << "Failed to create window!" << std::endl;
	}

	meh::init_renderer();

	if(!meh::load_engine_icons("engine_icons.png")){
	  std::cout << "failed to load engine icons!" << std::endl;
	}

	std::vector<MEH_TEXTURE_FONT> fonts;
	fonts.emplace_back(MEH_TEXTURE_FONT{"..\\Roboto-Bold.ttf", 14});
	fonts.emplace_back(MEH_TEXTURE_FONT{ "..\\Roboto-Regular.ttf", 18 });

	meh::create_font_texture(fonts,256,256, true,60);

	meh::draw_background();

	meh::widget btn1 = meh::button("some text!");
	btn1->X(40); // set x position
	btn1->Y(50); // set y position

	meh::widget btn2 = meh::button("some other text!");
	
	btn2->height(50);
	btn2->X(80);
	btn2->Y(90);

	meh::init_ui();
	
	double dt = 0.0;
	while (true) {
		if (!meh::update_window()) {
			break;
		}
		else {
			dt = meh::get_total_time();
			btn2->X(uint32_t(cos((dt * 2)) * 100 + 150));
			btn2->Y(uint32_t(sin((dt * 2)) * 100 + 150));
			meh::render();
			meh::calculate_delta_time();
		}
	}

	return meh::meh_cleanup();
}