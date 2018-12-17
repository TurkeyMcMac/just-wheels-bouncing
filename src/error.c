#define JWB_INTERNAL_
#include <jwb.h>

const char *jwb_errmsg(int code)
{
	if (code < 0) {
		code *= -1;
	}
	switch (code) {
	case JWBE_NO_MEMORY:
		return "Out of memory";
	case JWBE_REMOVED_ENTITY:
		return "Entity handle referenced a removed entity";
	case JWBE_DESTROYED_ENTITY:
		return "Entity handle referenced a destroyed entity (CRITICAL)";
	case JWBE_INVALID_ARGUMENT:
		return "Invalid argument";
	case 0:
		return "No error";
	default:
		return "Unknown error";
	}
}
