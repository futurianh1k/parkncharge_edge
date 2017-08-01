#include "LPR.h"

namespace seevider {
	using std::string;
	using std::vector;

	LPR::LPR() {
	}

	LPR::LPR(string country, std::string settings_filename) :
		mCountryCode(country)/*, mLPR(country, settings_filename)*/ {
	}

	string LPR::recognize(const cv::Mat& image, vector<string> &LPLists) const {
		/*alpr::AlprResults results = LPR.recognize(image.data, 3, image.cols, image.rows,
			std::vector<alpr::AlprRegionOfInterest>({ alpr::AlprRegionOfInterest(0, 0, image.cols, image.rows) }));
			
		for (int i = 0; i < results.plates.size(); i++) {
			alpr::AlprPlateResult plate = results.plates[i];
			std::cout << "plate" << i << ": " << plate.topNPlates.size() << " results" << std::endl;

			for (int k = 0; k < plate.topNPlates.size(); k++) {
				alpr::AlprPlate candidate = plate.topNPlates[k];
				LPLists.push_back(candidate.characters.c_str());	// candidate.overall_confidence;
				//std::cout << "\t pattern_match: " << candidate.matches_template << std::endl;
			}
		}*/

		if (!LPLists.empty()) {
			return LPLists.front();
		}
		else {
			return "null";
		}
	}
}
