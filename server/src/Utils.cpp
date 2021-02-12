#include "utils/Utils.h"

std::string& Utils::trim(std::string& str) {
    if (str.empty()) return str;
    str.erase(0, str.find_first_not_of(" "));
    str.erase(str.find_last_not_of(" ") + 1);
    return str;
}