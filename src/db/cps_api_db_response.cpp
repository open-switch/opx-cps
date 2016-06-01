
#include "cps_api_db_response.h"

#include "cps_string_utils.h"

#include <iostream>
#include <hiredis/hiredis.h>

#include <tuple>

int cps_db::response::get_int() {
	return static_cast<redisReply*>(_reply)->integer;
}

bool cps_db::response::is_status() {
	return static_cast<redisReply*>(_reply)->type == REDIS_REPLY_STATUS;
}

bool cps_db::response::has_elements() {
	return static_cast<redisReply*>(_reply)->type == REDIS_REPLY_ARRAY;
}
void *cps_db::response::element_at(size_t ix) {
	return static_cast<redisReply*>(_reply)->element[ix];
}
bool cps_db::response::is_str() {
	return static_cast<redisReply*>(_reply)->type == REDIS_REPLY_STRING;
}

size_t cps_db::response::elements()  {
	return static_cast<redisReply*>(_reply)->elements;
}

int cps_db::response::get_str_len()  {
	return static_cast<redisReply*>(_reply)->len;
}
const char *cps_db::response::get_str()  {
	return static_cast<redisReply*>(_reply)->str;
}

bool cps_db::response::is_int() {
	return static_cast<redisReply*>(_reply)->type == REDIS_REPLY_INTEGER;
}

bool cps_db::response::is_ok() {
	return static_cast<redisReply*>(_reply)->type != REDIS_REPLY_ERROR ;
}

void cps_db::response::iterator(const std::function<void(size_t ix, int type, const void *data, size_t len)> &fun) {
	if (static_cast<redisReply*>(_reply)->type != REDIS_REPLY_ARRAY) {
		return;
	}
	for (size_t ix = 0; ix < static_cast<redisReply*>(_reply)->elements ; ++ix ) {
		if (static_cast<redisReply*>(_reply)->element[ix]->type==REDIS_REPLY_INTEGER) {
			fun(ix,static_cast<redisReply*>(_reply)->element[ix]->type,
					&static_cast<redisReply*>(_reply)->element[ix]->integer,
					sizeof(static_cast<redisReply*>(_reply)->element[ix]->integer));
		}
		if (static_cast<redisReply*>(_reply)->element[ix]->type==REDIS_REPLY_STRING) {
			fun(ix,static_cast<redisReply*>(_reply)->element[ix]->type,
					static_cast<redisReply*>(_reply)->element[ix]->str,
					static_cast<redisReply*>(_reply)->element[ix]->len);
		}
	}
}

std::tuple<void *, size_t> cps_db::response::get_element(size_t ix) {
	STD_ASSERT(ix < static_cast<redisReply*>(_reply)->elements);
	return std::make_tuple(static_cast<redisReply*>(_reply)->element[ix]->str,static_cast<redisReply*>(_reply)->len);
}

size_t cps_db::response::extract_elements(response_element_t *lst, size_t max) {
	size_t _filled_len = 0;

	iterator([&](size_t ix, int type, const void *data, size_t len) {
			if (_filled_len >= max) return;
			lst[_filled_len] = std::make_tuple(type,data,len);
		});
	return _filled_len;
}

void cps_db::response::dump() {
	std::cout << "Resp type is : " << static_cast<redisReply*>(_reply)->type << std::endl;

	if(is_int()) {
		std::cout << "Int : " << get_int() << std::endl;
	}
	if (is_ok()) std::cout << "OK Resp..." << std::endl;

	if (is_str()) {
		std::cout << get_str() << std::endl;
	}
	if (has_elements()) {
		iterator([](size_t ix, int type, const void *data, size_t len) {
			std::cout << "IX:" << ix << " Type:" << type << " " << cps_string::tostring(data,len) << std::endl;
		});
	}
}
