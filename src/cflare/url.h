#ifndef CFLARE_URL_H
#define CFLARE_URL_H

#include <cflare/cflare.h>
#include <cflare/util.h>

typedef void(cflare_url_parse_callback)(const char* key, size_t key_len, const char* value, size_t value_len, void* context);

// parse query params like a=5&b=10 get translated into the calls `("a", "5")` and `("b", "10")`.
// "abc" will call `("abc", "")`, and "def&xyz", will call `("def")` and `("xyz")`.
CFLARE_API void cflare_url_parse_query(const char* input, size_t input_len, cflare_url_parse_callback* callback, void* context);

// an escaped url is always longer than it's unescaped counterpart, so a mutate is possible
// returns true if it mutated the string completely, else false
CFLARE_API bool cflare_unescape_url_mutate(char* input, size_t len, size_t* new_len);

#endif /* CFLARE_URL_H */

