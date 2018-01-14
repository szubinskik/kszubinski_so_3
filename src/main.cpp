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

int get_uidcount_from_select(std::vector<std::string> &v)
{
	for (auto e : v)
	{
		size_t found = e.find("EXISTS", 0);
		if (found != std::string::npos)
		{
			std::string buff = "";
			for (size_t i = 2; i < e.length(); ++i)
			{
				if (e[i] == ' ')
					return std::stoi(buff);

				buff += e[i];
			}
		}
	}
	return -1;
}

int get_uidcount(std::string dir)
{
	std::vector< std::string > v;
	imap_select(dir, handler_string_vector, &v);

	return get_uidcount_from_select(v);
}

std::vector<std::string> get_dirs()
{
	// get a list of directories
	std::vector<std::string> v;
	v.push_back("/");
	imap_list_all(handler_string_vector, &v);

	for(auto &e : v)
		e = get_path_from_list(e);

	return v;
}

bool v_find(std::vector<std::string> v, std::string s)
{
	for (auto e : v)
		if (e == s) return true;

	return false;
}


int do_rename(const char* from, const char* to)
{
	ERROR("do_rename(): " + std::string(to));
	auto v = get_dirs();

	std::string s_from = std::string(from);
	std::string s_to = std::string(to);

	if (v_find(v, s_from)) // if "from" is a dir
	{
		int status = imap_rename_dir(s_from, s_to);
		return (status == 0)?0:-ENOENT;
	}
	else // "from" is an e-mail
	{
		size_t index = std::string(s_from).rfind("/");
		if (index == std::string::npos)
			return -ENOENT;

		std::string uid = std::string(s_from).substr(index+1);
		std::string dir = std::string(s_from).erase(index);

		int i_uid;
		try
		{
			i_uid = std::stoi(uid);
		}
		catch(const std::exception& e)
		{
			return -ENOENT;
		}

		if (i_uid <= get_uidcount(dir))
		{
			int status = imap_move(s_from, s_to, i_uid);
			return (status == 0)?0:-ENOENT;
		}
	}

	return -ENOENT;
}

int do_getattr( const char *path, struct stat *st )
{
	TRACE("do_getattr(): "+std::string(path));

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

	auto v = get_dirs();

	// check if is on list
	// if so, it is a directory
	for(auto e : v)
	{
		if (e == std::string(path))
		{
			// describe entity as a directory
			st->st_mode = S_IFDIR | 0777;
			st->st_nlink = 0;
			return 0;
		}
	}

	// perhaps we're given a mail
	// if so, get its directory and check uidcount
	// firstly - trim out string
	size_t index = std::string(path).rfind("/");
	if (index == std::string::npos)
		return -ENOENT;

	std::string uid = std::string(path).substr(index+1);
	std::string dir = std::string(path).erase(index);

	// check for dir on list
	for(auto e : v)
	{
		if (e == dir)
		{
			// get uidcount of dir
			int uidcount = get_uidcount(dir);
			int i_uid;
			try
			{
				i_uid = std::stoi(uid);
			}
			catch(const std::exception& e)
			{
				return -ENOENT;
			}

			if (i_uid <= uidcount)
			{
				st->st_mode = S_IFREG | 0777;
				st->st_nlink = 0;
				return 0;
			}

			break;
		}
	}

	// no match - error
	return -ENOENT;
}

int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
						off_t offset, struct fuse_file_info *fi)
{
	TRACE("do_readdir(): "+std::string(path));
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

		v.clear();
		imap_select(std::string(path), handler_string_vector, &v);
		int uidcount = get_uidcount_from_select(v);
		for(int i = 1; i <= uidcount; ++i)
			filler( buffer, std::to_string(i).c_str(), NULL, 0 );
	}
	return 0;
}

int do_mkdir(const char *path, mode_t mode)
{
	TRACE("do_mkdir(): " + std::string(path));
	int status = imap_mkdir(std::string(path));
	if (status != 0)
		return -EIO;
	return 0;
}

int do_rmdir(const char *path)
{
	TRACE("do_rmdir(): " + std::string(path));
	int status = imap_rmdir(std::string(path));
	if (status != 0)
		return -EIO;
	return 0;
}

static struct fuse_operations operations;

int run_fuse(int argc, char* argv[])
{
	operations.getattr = do_getattr;
	operations.mkdir   = do_mkdir;
	operations.readdir = do_readdir;
	operations.rmdir   = do_rmdir;
	operations.rename  = do_rename;
	return fuse_main( argc, argv, &operations, NULL );
}

int main(int argc, char* argv[])
{
	init_curl(argv[1], argv[2]);

	debug_init(V_TRACE);
	run_fuse(argc-2, argv+2);

	return 0;
}