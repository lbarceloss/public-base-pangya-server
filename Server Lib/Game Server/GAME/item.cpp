
#if defined(_WIN32)
#pragma pack(1)
#endif

#include "item.hpp"
#include <memory>

#include <memory.h>

using namespace stdA;

item::item() {
	memset(this, 0, sizeof(item));
};

item::~item() {

};
