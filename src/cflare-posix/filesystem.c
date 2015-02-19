
#include "cflare/filesystem.h"

#include <dirent.h>
#include <stdlib.h>


void free_list(void* data, void* context)
{
	cflare_filesystem_entry* e = (cflare_filesystem_entry*)data;
	free(e->name);
}

void populate_list(cflare_linkedlist* list, const char* path, size_t path_len, size_t depth, uint8_t recursive, uint8_t exc_dirs)
{
	DIR* dir;
	struct dirent* ent;
	
	if(!(dir = opendir(path)))
	{
		cflare_warn("filesystem: could not list %s", path);
		return;
	}
	
	while(1)
	{
		ent = readdir(dir);
		if(!ent)
			break;
		
		size_t f_len = strlen(ent->d_name);
		
		if(f_len == 1 && ent->d_name[0] == '.')
			continue;
		if(f_len == 2 && ent->d_name[0] == '.' && ent->d_name[1] == '.')
			continue;
		
		cflare_filesystem_entry_type t;
		switch(ent->d_type)
		{
		case DT_UNKNOWN:
			t = CFLARE_FILESYSTEM_UNKNOWN;
			break;
		case DT_REG:
			t = CFLARE_FILESYSTEM_FILE;
			break;
		case DT_DIR:
			t = CFLARE_FILESYSTEM_DIRECTORY;
			break;
		case DT_FIFO:
			t = CFLARE_FILESYSTEM_FIFO;
			break;
		case DT_SOCK:
			t = CFLARE_FILESYSTEM_SOCKET;
			break;
		case DT_CHR:
			t = CFLARE_FILESYSTEM_CHARDEV;
			break;
		case DT_BLK:
			t = CFLARE_FILESYSTEM_BLOCKDEV;
			break;
		case DT_LNK:
			t = CFLARE_FILESYSTEM_LINK;
			break;
		default:
			t = CFLARE_FILESYSTEM_UNKNOWN;
			break;
		}
		
		if(!exc_dirs || (exc_dirs && t != CFLARE_FILESYSTEM_DIRECTORY))
		{
			cflare_filesystem_entry* f;
			cflare_linkedlist_insert_last(list, (void**)&f);
			
			size_t len = path_len + f_len + 1;
			char* name = malloc(len);
			
			memcpy(name, path, path_len);
			memcpy(name + path_len, ent->d_name, f_len);
			name[path_len + f_len] = '\0';
			
			f->name = name;
			f->type = t;
			f->depth = depth;
		}
		
		if(recursive && t == CFLARE_FILESYSTEM_DIRECTORY)
		{
			size_t newpath_len = path_len + f_len + 1; // +1 for slash
			char* newpath = malloc(newpath_len + 1); // +1 for \0
			
			memcpy(newpath, path, path_len);
			memcpy(newpath + path_len, ent->d_name, f_len);
			newpath[newpath_len - 1] = '/';
			newpath[newpath_len    ] = '\0';
			
			populate_list(list, newpath, newpath_len, depth + 1, recursive, exc_dirs);
			
			free(newpath);
		}
	}
	
	closedir(dir);
}

cflare_linkedlist* cflare_filesystem_list(const char* path, cflare_filesystem_listoptions opts)
{
	cflare_linkedlist* list = cflare_linkedlist_new(sizeof(cflare_filesystem_entry));
	cflare_linkedlist_ondelete(list, &free_list, 0);
	
	size_t path_len = strlen(path);
	char* pp = 0;
	
	if(path[path_len - 1] != '/')
	{
		// pp = path + "/";
		pp = malloc(path_len + 2);
		memcpy(pp, path, path_len);
		pp[path_len    ] = '/';
		pp[path_len + 1] = '\0';
		path_len += 1;
		
		path = pp;
	}
	
	populate_list(list, path, path_len, 0,
		!!(opts & CFLARE_FILESYSTEM_LIST_RECURSIVE),
		!!(opts & CFLARE_FILESYSTEM_LIST_EXCLUDE_DIRECTORIES)
	);
	
	if(pp)
		free(pp);
	
	return list;
}
