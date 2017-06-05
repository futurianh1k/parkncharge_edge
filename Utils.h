#pragma once

#include <string>

namespace seevider {
	/**
	 * Transform string to bool
	 */
	bool to_bool(std::string str);

	/**
	 * Transform bool to string
	 */
	std::string to_string(bool val);

	/**
	 * OnMouse event handler for ROI settings
	 */
	void ROISetting_OnMouse(int event, int x, int y, int, void *data);
}
