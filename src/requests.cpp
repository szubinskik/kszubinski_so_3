#include "imap.h"

#include <string>
#include <vector>
#include <cstring>

typedef size_t (*imap_handler)(char*, size_t, size_t, void*);

// handlers definitions

size_t handler_string_vector(char *ptr, size_t size, size_t nmemb, void *vector); // pushes response lines to std::string vector at @vector

// commands definitions

int imap_list_all(imap_handler handler, void *pointer);
int imap_list_subdirs(std::string dir, imap_handler handler, void *pointer);

// handlers implementation

size_t handler_string_vector(char *ptr, size_t size, size_t nmemb, void *vector)
{
	char *word = new char [256]; // TODO
	memcpy(word, ptr, size*nmemb);
	word[size*nmemb] = 0;

	auto s_vector = static_cast< std::vector<std::string>* >(vector);
	s_vector -> push_back( std::string(word) );

	delete[] word;
	return size*nmemb;
}

// commands implemenatation

int imap_list_all(imap_handler handler, void* pointer)
{
	//curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "LIST \"/\" \"*\"");
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "LIST \"/\" \"%\"");

	if (handler)
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handler_string_vector);

	if (pointer)
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, pointer);

	return _make_request();
}

int imap_list_subdirs(std::string dir, imap_handler handler, void *pointer)
{
	if (dir[dir.length()-1] != '/')
		dir += '/';

	std::string command = "LIST \"" + dir + "\" \"%\"";
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, command.c_str());

	if (handler)
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handler_string_vector);

	if (pointer)
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, pointer);

	return _make_request();
}