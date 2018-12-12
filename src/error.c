#include <jwb.h>

const char *jwb_errmsg(int code)
{
	if (code < 0) {
		code *= -1;
	}
	switch (code) {
	case JWBE_NO_MEMORY:
		return "Out of memory";
	case JWBE_INVALID_ENTITY:
		return "Invalid entity handle";
	case 0:
		return "No error";
	default:
		return "Unknown error";
	}
}
