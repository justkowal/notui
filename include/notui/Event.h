#pragma once
#include <string>
#include <any>

namespace notui {

struct Widget;

struct Event {
    Widget* sender = nullptr;
    std::string name;
    std::any data;
};

} 
