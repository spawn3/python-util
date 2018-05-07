#include <time.h>
#include <string.h>
#include <hiredis.h>


redisContext *redis_context = NULL;


int redis_init() {
	struct timeval timeout = {1, 500000};
	redis_context = redisConnectWithTimeout("127.0.0.1", 6379, timeout);
	return 0;
}


typedef struct {
	int a;
	int b;
	int c[20];
} payload_t;


int r_add(const char *name) {
	redisReply *reply;
	payload_t payload;

	payload.a = 1;
	payload.b = 2;

	reply = redisCommand(redis_context, "HSET fileid %s %b", name, &payload, sizeof(payload_t));
	freeReplyObject(reply);

	return 0;
}

int r_read(const char *name) {
	redisReply *reply;
	// payload_t *p;

	reply = redisCommand(redis_context, "HGET fileid %s", name);

	// p = (payload_t *)reply->str;
	// printf("read %s (%d, %d, %s (%d, %d))\n", name, reply->type, reply->len, reply->str, p->a, p->b);
	freeReplyObject(reply);

	return 0;
}

int r_hlen(const char *name) {
	redisReply *reply;

	reply = redisCommand(redis_context, "HLEN fileid");
	printf("hlen type %d, len %d, integer %lld\n", reply->type, reply->len, reply->integer);
	freeReplyObject(reply);
	return 0;
}

int test_hash2() {
	const char *name = "a";

	r_add(name);
	r_read(name);
	r_hlen(name);
	return 0;
}


int test_hash() {
	int i, j, count;
	char name[256];
	char cursor[256];
	printf("...enter test_hash\n");
	redisReply *reply, *p;
	// payload_t *payload;

	for (i=0; i<1000; ++i) {
		sprintf(name, "%d", i);
		r_add(name);
		//r_read(name);
	}

	sprintf(cursor, "%d", 0);
	count = 0;
	while (1) {
		// printf("cursor %s\n", cursor);
		reply = redisCommand(redis_context, "HSCAN fileid %s", cursor);
		if (reply->type == REDIS_REPLY_ARRAY) {
			p = reply->element[0];
			printf("%4d: cursor %s -> %s\n", count, cursor, p->str);
			memcpy(cursor, p->str, strlen(p->str)+1);

			p = reply->element[1];
			for (i = 0; i < p->elements; ++i) {
				if (i % 2 == 0) {
					// printf("reply %d (%d, %d, %s)\n", i, p->type, p->len, p->element[i]->str);
					count += 1;
				} else {
					// payload = (payload_t *)p->element[i]->str;
					// printf("reply %d (%d, %d, %d:%d)\n", i, p->type, p->len, payload->a, payload->b);
				}
			}

			if (strcmp(cursor, "0") == 0) 
				break;

			for (j = 0; j < reply->elements; j++) {
				p = reply->element[j];
				// printf("j = %d, %d\n", j, p->type);
				if (p->type == REDIS_REPLY_STRING) {
					// printf("reply cursor (%d, %d, %s)\n", p->type, p->len, p->str);
				} else if (p->type == REDIS_REPLY_ARRAY) {
					for (i = 0; i < p->elements; ++i) {
						if (i % 2 == 0) {
							// printf("reply %d (%d, %d, %s)\n", i, p->type, p->len, p->element[i]->str);
						} else {
							// payload = (payload_t *)p->element[i]->str;
							// printf("reply %d (%d, %d, %d:%d)\n", i, p->type, p->len, payload->a, payload->b);
						}
					}
				}

			}
		}
		freeReplyObject(reply);
	}
	printf("total = %d\n", count);


	return 0;
}


int main(int argc, char *argv[]) {
	redis_init();
	test_hash2();

	return 0;
}
