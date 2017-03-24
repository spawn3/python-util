#include <uuid/uuid.h>

int add(int x, int y)
{
	uuid_t uuid;

	uuid_generate(uuid);

	return x + y;
}
