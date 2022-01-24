#include "imgui_util.h"

std::string labelize(long long int id, std::string label, long long int second_id) {
    if (label.empty()) {
        return std::string("##label") + std::to_string(id) + std::string("_") + std::to_string(second_id);
    }
    else {
        return label + std::string("##") + label + std::to_string(id) + std::string("_") + std::to_string(second_id);
    }
}