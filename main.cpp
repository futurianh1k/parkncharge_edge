#include "main.h"

#include "MainInterface.h"
#include "IOUtils.h"

#include <boost/property_tree/xml_parser.hpp>
#include <glog/logging.h>

using namespace seevider;

bool system_check() {
	// Check if the system core folder is exist
	if (!utils::checkFolder(SYSTEM_FOLDER_CORE)) {
		std::cout << "Failed to create the system folder: " << SYSTEM_FOLDER_CORE << std::endl;
		return false;
	}

	if (!utils::checkFolder(SYSTEM_FOLDER_LOG)) {
		std::cout << "Failed to create the system folder: " << SYSTEM_FOLDER_LOG << std::endl;
		return false;
	}

	// TODO: do or check anything that is required to do before starting the system.

	return true;
}

void writeXMLFile(cv::string filename) {
	using boost::property_tree::ptree;

	ptree root;
	ServerDestinations_t dest;

	dest.RequestType = HTTP_REQ_SYNC_GENERAL;
	dest.HTTPRequestMethod = "PUT";
	dest.TargetPath = "/parksync/sync";
	ServerDestinations[HTTP_REQ_SYNC_GENERAL] = dest;

	dest.RequestType = HTTP_REQ_UPDATE_ENTER;
	dest.HTTPRequestMethod = "POST";
	dest.TargetPath = "/parkstatus/enter";
	ServerDestinations[HTTP_REQ_UPDATE_ENTER] = dest;

	dest.RequestType = HTTP_REQ_UPDATE_EXIT;
	dest.HTTPRequestMethod = "POST";
	dest.TargetPath = "/parkstatus/exit";
	ServerDestinations[HTTP_REQ_UPDATE_EXIT] = dest;

	dest.RequestType = HTTP_REQ_UPDATE_OVER;
	dest.HTTPRequestMethod = "POST";
	dest.TargetPath = "/parkstatus/overtime";
	ServerDestinations[HTTP_REQ_UPDATE_OVER] = dest;

	root.add("ServerSettings.ServerAddress", "52.52.203.176");
	for (const auto& data : ServerDestinations) {
		ptree &node = root.add("ServerSettings.Request", "");

		node.put("RequestType", data.second.RequestType);
		node.put("HTTPRequestMethod", data.second.HTTPRequestMethod);
		node.put("TargetPath", data.second.TargetPath);
	}

	write_xml(filename, root, std::locale(), boost::property_tree::xml_writer_make_settings<std::string >('\t', 1));
}

void loadXMLFile(std::string filename) {
	using boost::property_tree::ptree;

	ptree root;
	boost::property_tree::read_xml(filename, root);

	for (ptree::value_type const& v : root.get_child("ServerSettings")) {
		if (v.first == "ServerAddress") {
			std::cout << "IP address: " << v.second.get_value<std::string>() << std::endl;
		}
		else if (v.first == "Request") {
			ServerDestinations_t dest;

			dest.RequestType = v.second.get<int>("RequestType");
			dest.HTTPRequestMethod = v.second.get<std::string>("HTTPRequestMethod");
			dest.TargetPath = v.second.get<std::string>("TargetPath");
			ServerDestinations[dest.RequestType] = dest;
		}
	}

	for (auto const& elem : ServerDestinations) {
		std::cout << "(" << elem.second.RequestType << ")";
		std::cout << " " << elem.second.HTTPRequestMethod << ",";
		std::cout << " " << elem.second.TargetPath << std::endl;
	}
}

int main(int argc, char** argv) {
	if (!system_check()) {
		std::cerr << "Failed to check system" << std::endl;
	}

	// Initialize Google's logging library.
	FLAGS_alsologtostderr = 1;
	FLAGS_log_dir = "./log/";
	google::InitGoogleLogging(argv[0]);

    MainInterface mainInterface;

    mainInterface.run();
}
