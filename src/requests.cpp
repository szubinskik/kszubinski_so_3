#include "imap.h"

#include <string>
#include <vector>
#include <cstring>

typedef size_t (*imap_handler)(char*, size_t, size_t, void*);

// handlers implementation

size_t handler_string_vector(char *ptr, size_t size, size_t nmemb, void *vector)
{
	char *word = new char [size*nmemb+1]; // TODO
	memcpy(word, ptr, size*nmemb);
	word[size*nmemb] = 0;

	auto s_vector = static_cast< std::vector<std::string>* >(vector);
	s_vector -> push_back( std::string(word) );

	delete[] word;
	return size*nmemb;
}

size_t handler_string(char *ptr, size_t size, size_t nmemb, void *result)
{
	char *word = new char [size*nmemb+1]; // TODO
	memcpy(word, ptr, size*nmemb);
	word[size*nmemb] = 0;

	auto s_result = static_cast< std::string* >(result);
	s_result->append(word);

	delete[] word;
	return size*nmemb;
}

// commands implemenatation

int imap_search_all(std::string path, imap_handler handler, void* pointer)
{
	CURL *curl = open_curl();
	if (path[0] == '/')
		path.erase(0, 1);

	std::string command;
	int status;

	if (handler)
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handler);

	if (pointer)
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, pointer);

	command = "SELECT \"" + path + "\"";
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, command.c_str());
	status = _make_request(curl);
	if (status != 0)
	{
		close_curl(curl);
		return status;
	}

	command = "UID SEARCH ALL";
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, command.c_str());
	status = _make_request(curl);

	close_curl(curl);
	return status;
}

int imap_move(std::string from, std::string to, int uid, imap_handler handler, void* pointer)
{
	CURL *curl = open_curl();
	if(to[0] == '/')
		to.erase(0, 1);

	if(from[0] == '/')
		from.erase(0, 1);


	if (handler)
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handler);

	if (pointer)
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, pointer);

	std::string command;
	int status;

	command = "SELECT " + from;
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, command.c_str());
	status = _make_request(curl);
	if (status != 0)
	{
		close_curl(curl);
		return status;
	}

	command = "UID MOVE " + std::to_string(uid) + " " + to;
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, command.c_str());
	status = _make_request(curl);

	close_curl(curl);
	return status;
}

int imap_rename_dir(std::string from, std::string to,imap_handler handler, void* pointer)
{
	CURL *curl = open_curl();
	if(to[0] == '/')
		to.erase(0, 1);

	if(from[0] == '/')
		from.erase(0, 1);

	std::string command = "RENAME " + from + " " + to;
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, command.c_str());

	if (handler)
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handler);

	if (pointer)
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, pointer);

	int status = _make_request(curl);
	close_curl(curl);
	return status;
}

int imap_uid_to_ms(std::string path, int uid, imap_handler handler, void* pointer)
{
	CURL *curl = open_curl();
	if(path[0] == '/')
		path.erase(0, 1);

	if (handler)
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handler);

	if (pointer)
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, pointer);

	std::string command;
	int status;

	command = "SELECT " + path;
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, command.c_str());
	status = _make_request(curl);
	if (status != 0)
	{
		close_curl(curl);
		return status;
	}

	command = "SEARCH UID " + std::to_string(uid);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, command.c_str());
	status = _make_request(curl);

	close_curl(curl);
	return status;
}

// some problems due to CURL#536
// need to use URL-base command, instead of custom request
int imap_fetch_mail(std::string mailbox, unsigned int uid, imap_handler handler, void* pointer)
{
	CURL *curl = open_curl();
	if(mailbox[0] == '/')
		mailbox.erase(0, 1);

	char *_enc_box = curl_easy_escape(curl, mailbox.c_str(), 0);
	std::string enc_box = std::string(_enc_box);
	delete _enc_box;

	std::string command = "imaps://imap.gmail.com:993/" + enc_box + "/;UID=" + std::to_string(uid) + ";SECTION=TEXT";
	curl_easy_setopt(curl, CURLOPT_URL, command.c_str());

	if (handler)
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handler);

	if (pointer)
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, pointer);

	int status = _make_request(curl);
	close_curl(curl);
	return status;
}

int imap_fetch_size(std::string mailbox, unsigned int uid, imap_handler handler, void* pointer)
{
	CURL *curl = open_curl();
	if(mailbox[0] == '/')
		mailbox.erase(0, 1);

	char *_enc_box = curl_easy_escape(curl, mailbox.c_str(), 0);
	std::string enc_box = std::string(_enc_box);
	delete _enc_box;

	std::string command;
	int status;

	command = "SELECT \"" + mailbox + "\"";
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, command.c_str());

	if (handler)
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handler);

	if (pointer)
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, pointer);

	status = _make_request(curl);
	if (status != 0)
		return status;

	command = "FETCH " + std::to_string(uid) + " BODY.PEEK[TEXT]";
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, command.c_str());
	status = _make_request(curl);
	close_curl(curl);
	return status;
}

int imap_select(std::string path, imap_handler handler, void* pointer)
{
	CURL *curl = open_curl();
	if (path[0] == '/')
		path.erase(0, 1);

	std::string command = "SELECT \"" + path + "\"";
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, command.c_str());

	if (handler)
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handler);

	if (pointer)
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, pointer);

	int status = _make_request(curl);
	close_curl(curl);
	return status;
}

int imap_list_all(imap_handler handler, void* pointer)
{
	CURL *curl = open_curl();
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "LIST \"/\" \"*\"");

	if (handler)
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handler);

	if (pointer)
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, pointer);

	int status = _make_request(curl);
	close_curl(curl);
	return status;
}

int imap_rmdir(std::string path, imap_handler handler, void* pointer)
{
	CURL *curl = open_curl();
	if (path[0] == '/')
		path.erase(0, 1);

	std::string command = "DELETE \"" + path + "\"";
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, command.c_str());

	if (handler)
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handler);

	if (pointer)
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, pointer);

	int status = _make_request(curl);
	close_curl(curl);
	return status;
}

int imap_mkdir(std::string path, imap_handler handler, void* pointer)
{
	CURL *curl = open_curl();
	if (path[0] == '/')
		path.erase(0, 1);

	std::string command = "CREATE \"" + path + "\"";
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, command.c_str());

	if (handler)
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handler);

	if (pointer)
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, pointer);

	int status = _make_request(curl);
	close_curl(curl);
	return status;
}

int imap_list_subdirs(std::string dir, imap_handler handler, void *pointer)
{
	CURL *curl = open_curl();
	if (dir[dir.length()-1] != '/')
		dir += '/';

	std::string command = "LIST \"" + dir + "\" \"%\"";
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, command.c_str());

	if (handler)
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handler);

	if (pointer)
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, pointer);

	int status = _make_request(curl);
	close_curl(curl);
	return status;
}