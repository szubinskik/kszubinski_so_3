#define FUSE_USE_VERSION 30
#include <cstring>
#include <cstdlib>
#include <vector>
#include <fuse.h>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <string>

#include "imap.h"
#include "filetree.h"
#include "debug.h"

std::vector<std::string> split_path(std::string path)
{
	std::vector<std::string> res;
	size_t i = 0;
	while (i < path.length())
	{
		std::string temp = "";
		while (i < path.length())
		{
			if (path[i] == '/')
				break;

			temp += path[i++];
		}
		res.push_back(temp);
		i++;
	}
	return res;
}

std::string get_path_from_list(std::string line)
{
	std::string res = "/";
	size_t i = 0;
	while (i < line.length() && line[i] != '"')
		i++;
	i += 5;
	while (i < line.length() && line[i] != '"')
		res += line[i++];
	return res;
}

int do_getattr( const char *path, struct stat *st )
{
	LOG("do_getattr(): "+std::string(path));

	// GNU's definitions of the attributes (http://www.gnu.org/software/libc/manual/html_node/Attribute-Meanings.html):
	// 		st_uid: 	The user ID of the file’s owner.
	//		st_gid: 	The group ID of the file.
	//		st_atime: 	This is the last access time for the file.
	//		st_mtime: 	This is the time of the last modification to the contents of the file.
	//		st_mode: 	Specifies the mode of the file. This includes file type information (see Testing File Type) and the file permission bits (see Permission Bits).
	//		st_nlink: 	The number of hard links to the file. This count keeps track of how many directories have entries for this file. If the count is ever decremented to zero, then the file itself is discarded as soon 
	//						as no process still holds it open. Symbolic links are not counted in the total.
	//		st_size:	This specifies the size of a regular file in bytes. For files that are really devices this field isn’t usually meaningful. For symbolic links this specifies the length of the file name the link refers to.

	st->st_uid = getuid(); // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
	st->st_atime = time( NULL ); // The last "a"ccess of the file/directory is right now
	st->st_mtime = time( NULL ); // The last "m"odification of the file/directory is right now


	// check if is on list
	// for now, are filesystem deals with directories only,
	// so not being at dir list means not existing
	std::vector<std::string> v;
	v.push_back("/");
	imap_list_all(handler_string_vector, &v);
	bool matched = false;
	for(auto e : v)
	{
		if (get_path_from_list(e) == std::string(path))
		{
			matched = true;
			break;
		}
	}
	if (!matched)
	{
		ERROR("directory " + std::string(path) + " doesn't exist");
		return -ENOENT;
	}


	// for now only structures are directories with no
	// symlinks and 0766 permissions
	st->st_mode = S_IFDIR | 0777;
	st->st_nlink = 0;

	return 0;
}

int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
						off_t offset, struct fuse_file_info *fi)
{
	LOG("do_readdir(): "+std::string(path));
	filler( buffer, ".", NULL, 0 );
	filler( buffer, "..", NULL, 0 );

	if ( true )//strcmp( path, "/" ) == 0 )
	{
		std::vector<std::string> v;
		imap_list_subdirs(std::string(path), handler_string_vector, &v);
		for(auto path : v)
		{
			const char* p = split_path(get_path_from_list(path)).back().c_str();
			filler( buffer, p, NULL, 0 );
		}
		//filler( buffer, "file349", NULL, 0 );
	}
	return 0;
}

int do_mkdir(const char *path, mode_t mode)
{
	LOG("do_mkdir(): " + std::string(path));
	imap_mkdir(std::string(path));
	return 0;
}

int do_rmdir(const char *path)
{
	LOG("do_rmdir(): " + std::string(path));
	imap_rmdir(std::string(path));
	return 0;
}

static struct fuse_operations operations;

int run_fuse(int argc, char* argv[])
{
	operations.getattr = do_getattr;
	operations.mkdir   = do_mkdir;
	operations.readdir = do_readdir;
	operations.rmdir   = do_rmdir;
	return fuse_main( argc, argv, &operations, NULL );
}

int main(int argc, char* argv[])
{
	init_curl(argv[1], argv[2]);
	//curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "LIST \"/\" \"*\"");
	//_make_request();
	debug_init(V_LOG);
	run_fuse(argc-2, argv+2);

	return 0;
}