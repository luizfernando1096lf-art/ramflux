#pragma once

#include <string>

namespace core {

struct BuildInfo {
    static std::string gitHash();
};

}
