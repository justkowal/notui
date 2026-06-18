#include "notui/Spacer.h"

namespace notui {

Spacer::Spacer() { 
    flex = 1; 
}

void Spacer::render() { 
    style.apply(plane);
    ncplane_erase(plane); 
}

} 
