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
#include <map>
#include <cassert>
#include <utility>

#include "imap.h"
#include "debug.h"

typedef std::pair<std::string, std::string> File;

// utility functions
File get_file(std::string path);
File get_file(char *path);
bool is_file(File file);

// imap requests
std::string get_path_from_list(std::string line);
std::vector<std::string> get_dirs();
std::vector<int> get_uids(std::string dir);

// parsers for imap commands
std::vector<int> parse_search_all(std::vector<std::string> v);
int parse_get_size(std::vector<std::string> v);


// global variables

std::map<std::string, std::string> path_to_content;

// split path to dir and file
File get_file(std::string path)
{
	size_t index = std::string(path).rfind("/");
	if (index == std::string::npos)
		return { "\\", "\\" };

	std::string dir = std::string(path).erase(index);
	std::string file = std::string(path).substr(index+1);

	return {dir, file};
}

File get_file(char *path)
{
	return get_file(std::string(path));
}

bool is_file(File file)
{
	return !(file.first == "\\" && file.second == "\\");
}

// search vector for an element
template<class T>
bool v_find(std::vector<T> v, T s)
{
	for (auto e : v)
		if (e == s) return true;

	return false;
}

// get the list of directories
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

// get all uids in directory
std::vector<int> get_uids(std::string dir)
{
	std::vector<std::string> v;
	imap_search_all(dir, handler_string_vector, &v);
	return parse_search_all(v);
}

std::vector<int> parse_search_all(std::vector<std::string> v)
{
	std::vector<int> res;
	const std::string pattern = "* SEARCH";

	for (auto e : v)
	{
		if (e.length() < pattern.length())
			continue;

		if (std::equal(pattern.begin(), pattern.end(), e.begin()))
		{
			e += ' ';
			std::string buff = "";
			for (size_t i = pattern.length()+1; i < e.length(); ++i)
			{
				if (e[i] == ' ')
				{
					try { res.push_back(std::stoi(buff)); } catch (const std::exception& e) {}
					buff = "";
				}
				else
					buff += e[i];
			}

			break;
		}
	}

	return res;
}

int parse_get_size(std::vector<std::string> v)
{
	int res = -1;
	if (v.empty())
		return res;

	auto e = v.back();
	size_t index = e.length();
	for (size_t i = 6; i < e.length(); ++i)
	{
		if (e[i] == '{')
		{
			index = i+1;
			break;
		}
	}

	std::string temp = "";
	for (; index < e.length(); ++index)
	{
		if (e[index] == '}')
			break;
		temp += e[index];
	}

	try { res = std::stoi(temp); } catch (const std::exception& e) {}
	return res;
}

// FUSE handlers implementation

int do_open(const char* path, struct fuse_file_info* fi)
{
	auto s_path = std::string(path);
	TRACE("do_open(): " + s_path);

	File f = get_file(s_path);
	if (!is_file(f))
		return -ENOENT;

	int uid;
	try { uid = std::stoi(f.second); } catch (const std::exception& e) { return -ENOENT; }

	// convert UID to ms
	std::vector<std::string> v;
	imap_uid_to_ms(f.first, uid, handler_string_vector, &v);

	// TODO
	int ms = parse_search_all(v).back();

	std::string res = "";
	imap_fetch_mail(f.first, ms, handler_string, &res);

	path_to_content[s_path] = res;
	return 0;
}

int do_read(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
	auto s_path  = std::string(path);
	TRACE("do_read(): " + s_path + " size: " + std::to_string(size) + " offset: " + std::to_string(offset));
	auto content = path_to_content[s_path];
	size_t len = content.length();

	if (offset >= len)
		return 0;

	if (size > len - offset)
		size = len - offset;

	memcpy(buf, content.c_str() + offset, size);
	return static_cast<int>(size);
}

int do_rename(const char* from, const char* to)
{
	TRACE("do_rename(): " + std::string(from) + " " + std::string(to));
	auto v = get_dirs();

	std::string s_from = std::string(from);
	std::string s_to = std::string(to);

	if (v_find(v, s_from)) // if "from" is a dir
	{
		int status = imap_rename_dir(s_from, s_to);
		return (status == 0)?0:-ENOENT;
	}
	else // "from" is an e-mail
		 // for now, user cannot specify filename
	{
		File to_file = get_file(s_to);
		if (!is_file(to_file))
			return -ENOENT;

		File from_file = get_file(s_from);
		if (!is_file(from_file))
			return -ENOENT;

		int uid;
		try { uid = std::stoi(from_file.second); } catch (const std::exception& e) { return -ENOENT; }

		int status = imap_move(from_file.first, to_file.first, uid);
		return (status == 0)?0:-ENOENT;
	}

	return -ENOENT;
}

int do_getattr( const char *path, struct stat *st )
{
	TRACE("do_getattr(): "+std::string(path));

	st->st_uid = getuid(); // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
	st->st_atime = time( NULL ); // The last "a"ccess of the file/directory is right now
	st->st_mtime = time( NULL ); // The last "m"odification of the file/directory is right now

	auto v = get_dirs();

	// check if is on list
	// if so, it is a directory
	if (v_find(v, std::string(path)))
	{
		// describe entity as a directory
		st->st_mode = S_IFDIR | 0777;
		st->st_nlink = 0;
		return 0;
	}

	// perhaps we're given a mail
	// if so, get its directory and check uidcount
	// firstly - trim out string
	File f = get_file(path);
	if (!is_file(f))
		return -ENOENT;

	int uid;
	try { uid = std::stoi(f.second); } catch (const std::exception& e) { return -ENOENT; }

	// check for dir on list
	if (v_find(v, f.first))
	{
		auto uids = get_uids(f.first);
		for (auto u : uids)
		{
			if (uid == u)
			{
				std::vector<std::string> v;
				imap_uid_to_ms(f.first, uid, handler_string_vector, &v);
				int ms = parse_search_all(v).back();

				v.clear();
				imap_fetch_size(f.first, ms, handler_string_vector, &v);
				int size = parse_get_size(v);
				if (size > 0)
					st->st_size = size;
				st->st_mode = S_IFREG | 0777;
				st->st_nlink = 0;
				return 0;
			}
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

	std::vector<std::string> v;
	imap_list_subdirs(std::string(path), handler_string_vector, &v);
	for(auto path : v)
	{
		const char* p = get_file(get_path_from_list(path)).second.c_str();
		filler( buffer, p, NULL, 0 );
	}

	auto uids = get_uids(std::string(path));
	for (auto u : uids)
		filler( buffer, std::to_string(u).c_str(), NULL, 0 );

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


// fuse handlers
static struct fuse_operations operations;

int run_fuse(int argc, char* argv[])
{
	operations.getattr = do_getattr;
	operations.mkdir   = do_mkdir;
	operations.readdir = do_readdir;
	operations.rmdir   = do_rmdir;
	operations.rename  = do_rename;
	operations.open    = do_open;
	operations.read    = do_read;
	return fuse_main( argc, argv, &operations, NULL );
}

// args parsing
static struct options {
	const char *username;
	const char *password;
	const char *server;
	const char *resolve;
	int show_help;
	VERBOSITY verbosity;
} options;

#define OPTION(t, p) \
	{ t, offsetof(struct options, p), 1 }

static const struct fuse_opt option_spec[] = {
	OPTION("--username=%s", username),
	OPTION("-u %s", username),
	OPTION("--password=%s", password),
	OPTION("-p %s", password),
	OPTION("--server=%s", server),
	OPTION("-a %s", server),
	OPTION("--resolve %s", resolve),
	OPTION("-r %s", resolve),
	OPTION("--verbosity=%d", verbosity),
	OPTION("--help", show_help),
	OPTION("-h", show_help),
	FUSE_OPT_END
};

static void show_help()
{
	printf("usage:  mountpoint [options]\n\n");
	printf("File-system specific options:\n"
		   "    --username=<s>, -u <s>          imap server username\n"
		   "    --password=<s>, -p <s>          imap server password\n"
		   "    --server=<s>, -a <s>            address of imap server\n"
		   "\n"
		   "    --resolve=<s>, -r <s>           force resolve to selected address\n"
		   "    --verbosity=<0|1|2>             verbosity: ERROR, LOG, TRACE\n"
		   "\n");
}


// main for initialisation and command handling
int main(int argc, char* argv[])
{
	options.password = strdup("");
	options.username = strdup("");
	options.server = strdup("");
	options.resolve = strdup("");
	options.verbosity = V_TRACE;
	fuse_args args = FUSE_ARGS_INIT(argc, argv);

	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return 1;

	if (options.show_help)
	{
		show_help();
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0] = (char*) "";
		run_fuse(args.argc, args.argv);
		return 0;
	}

	if (strcmp(options.password, "") == 0 || strcmp(options.username, "") == 0)
	{
		std::cout << "Invalid username and/or password. See --help for more information." << std::endl;
		return 1;
	}

	if (strcmp(options.server, "") == 0)
	{
		std::cout << "Invalid server address. See --help for more information." << std::endl;
		return 1;
	}

	if (strcmp(options.resolve, "") == 0)
		init_curl(options.username, options.password, options.server);
	else
		init_curl(options.username, options.password, options.server, options.resolve);

	debug_init(options.verbosity);
	run_fuse(args.argc, args.argv);
	return 0;
}