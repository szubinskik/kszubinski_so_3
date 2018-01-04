#ifndef IMAP_H
#define IMAP_H

#include <curl/curl.h>

typedef size_t (*imap_handler)(char*, size_t, size_t, void*);

extern CURL *curl;

// initializing libcurl
void init_curl(char* login, char* passwd);

// handlers definitions
size_t handler_string_vector(char *ptr, size_t size, size_t nmemb, void *vector);

// commands definitions
int imap_list_all(imap_handler handler = nullptr, void *pointer = nullptr);

// internal command for pushing request
int _make_request();

#endif /* IMAP_H */