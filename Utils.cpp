#include "Utils.h"
#include "types.h"

#include <algorithm>
#include <sstream>
#include <opencv2/opencv.hpp>

namespace seevider {
	bool to_bool(std::string str) {
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		std::istringstream is(str);
		bool b;
		is >> std::boolalpha >> b;
		return b;
	}

	std::string to_string(bool val) {
		if (val) {
			return "true";
		}
		else {
			return "false";
		}
	}

	void ROISetting_OnMouse(int event, int x, int y, int, void *data)
	{
		ROISettingCallbackData *callbackData = (ROISettingCallbackData *)data;

		// Action when left button is clicked
		if (event == CV_EVENT_LBUTTONDOWN) {
			if (!callbackData->start_draw)
			{
				callbackData->roi_x0 = x;
				callbackData->roi_y0 = y;
				callbackData->start_draw = true;
			}
			else {
				callbackData->roi_x1 = x;
				callbackData->roi_y1 = y;
				callbackData->start_draw = false;
				callbackData->roi_set = true;
			}
		}

		// Action when mouse is moving
		if ((event == CV_EVENT_MOUSEMOVE) && callbackData->start_draw) {
			// Redraw bounding box for annotation
			cv::Mat current_view;
			callbackData->image.copyTo(current_view);
			rectangle(current_view, cv::Point(callbackData->roi_x0, callbackData->roi_y0), cv::Point(x, y), cv::Scalar(0, 255, 0));
			imshow(callbackData->window_name, current_view);
		}

		if ((event == CV_EVENT_LBUTTONUP) && callbackData->start_draw) {
			callbackData->roi_x1 = x;
			callbackData->roi_y1 = y;
			callbackData->start_draw = false;
			callbackData->roi_set = true;
		}
	}

}
