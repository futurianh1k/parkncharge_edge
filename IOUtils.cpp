#include "IOUtils.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace seevider::utils;
using boost::property_tree::ptree;

/**
 * Load application options from given file
 * @params filename The name of file to load the options.
 */
bool loadOptions(const std::string filename) {
    return true;
}

/**
 * Save application options to given file
 * @params filename The name of file to save the options.
 */
void saveOptions(const std::string filename) {
    ptree ptRoot;   // root structure
    ptree ptServer; // store server settings
    ptree ptSensor; // store sensor settings

    // Construct data array for server settings
    ptServer.put("IP", "000.000.000.000");
    ptServer.put("PORT", "00000");

    // Construct data array for sensor settings
    ptSensor.put("ROI", 0);

    write_json(filename, ptRoot);
}