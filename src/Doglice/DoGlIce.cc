#include "DoGlIce.hpp"

DoGlIce::DoGlIce() : AppBase() {}

bool DoGlIce::init() {
    if (!AppBase::init())
        return false;

    return true;
}