#pragma once

#include <string>

namespace seevider {
    namespace utils {
        /**
        * Load application options from given file
        */
        bool loadOptions(const std::string filename);

        /**
        * Save application options to given file
        */
        void saveOptions(const std::string filename);
    }
}
