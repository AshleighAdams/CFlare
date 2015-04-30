
#include "cflare/url.h"

void cflare_url_parse_query(const char* input, size_t input_len, cflare_url_parse_callback* callback, void* context)
{
	if(!input)
		return;
	
	char* buffer_key = malloc(64);
	char* buffer_val = malloc(64);
	
	size_t buffer_key_len = 64;
	size_t buffer_val_len = 64;
	
	// works in the method "%" hex hex
	
	for(size_t i = 0; i < input_len; i++)
	{
		size_t key_len = 0;
		size_t val_len = 0;
		
		// read the key
		while(i < input_len)
		{
			if(input[i] == '&' || input[i] == '=')
				break;
			buffer_key[key_len] = input[i];
			key_len++;
			i++;
			
			// reallocate snippit
			if(key_len == buffer_key_len) // we need to grow the key buffer
			{
				buffer_key_len *= 2;
				cflare_debug("url_parse_query(): growing key buffer length to %"FMT_SIZE, buffer_key_len);
				
				void* new_ptr = realloc(buffer_key, buffer_key_len);
				if(!new_ptr)
					cflare_fatal("url_parse_query(): could not grow buffer!");
				buffer_key = new_ptr;
			}
		}
		buffer_key[key_len] = '\0';
		
		// read the value if it exists
		if(i < input_len && input[i] == '=')
		{
			i++;
			while(i < input_len)
			{
				if(input[i] == '&')
					break;
				buffer_val[val_len] = input[i];
				val_len++;
				i++;
				
				// reallocate snippit
				if(val_len == buffer_val_len)
				{
					buffer_val_len *= 2;
					cflare_debug("url_parse_query(): growing value buffer length to %"FMT_SIZE, buffer_key_len);
					
					void* new_ptr = realloc(buffer_val, buffer_val_len);
					if(!new_ptr)
						cflare_fatal("url_parse_query(): could not grow buffer!");
					buffer_val = new_ptr;
				}
			}
		}
		buffer_val[val_len] = '\0';
		
		// now we have the escaped key and value, unescape_url_mutate them, and call the callback
		cflare_unescape_url_mutate(buffer_key, key_len, &key_len);
		cflare_unescape_url_mutate(buffer_val, val_len, &val_len);
		
		callback(buffer_key, key_len, buffer_val, val_len, context);
	}
	
	free(buffer_key);
	free(buffer_val);
}


bool cflare_unescape_url_mutate(char* input, size_t len, size_t* new_len)
{
	// subtract 2 is because the format is in "%FF" only
	size_t i, n;
	for(i = 0, n = 0; i < len; i++, n++)
	{
		switch(input[i])
		{
			case '%':
			{
				if(i + 2 >= len)
				{
					cflare_log("unescape_url_mutate(): input string has badly formatted escape: %s", input);
					return false;
				}
				
				char buff[] = {'0', 'x', input[i + 1], input[i + 2], '\0'};
				
				
				int64_t ascii;
				if(cflare_tointeger(buff, &ascii))
					input[n] = (char)ascii;
				else
				{
					cflare_log("unescape_url_mutate(): input string has bad ASCII escape: %s", buff);
					return false;
				}
				i += 2;
			} break;
		case '+': // this is pretty common...
			input[n] = ' ';
			break;
		default:
			input[n] = input[i];
			break;
		}
	}
	
	*new_len = n;
	input[n] = '\0';
	return true;
}
