#ifndef IMAP_H
#define IMAP_H

#include <curl/curl.h>
#include <string>

typedef size_t (*imap_handler)(char*, size_t, size_t, void*);

// initializing libcurl
void init_curl(char* login, char* passwd);

// handlers definitions
size_t handler_string_vector(char *ptr, size_t size, size_t nmemb, void *vector);
size_t handler_string(char *ptr, size_t size, size_t nmemb, void *result);

// commands definitions
int imap_uid_to_ms(std::string path, int uid, imap_handler handler = nullptr, void* pointer = nullptr);
int imap_search_all(std::string path, imap_handler handler = nullptr, void* pointer = nullptr);
int imap_move(std::string from, std::string to, int uid, imap_handler handler = nullptr, void* pointer = nullptr);
int imap_rename_dir(std::string from, std::string to,imap_handler handler = nullptr, void* pointer = nullptr);
int imap_fetch_mail(std::string mailbox, unsigned int uid, imap_handler handler = nullptr, void* pointer = nullptr);
int imap_select(std::string path, imap_handler handler = nullptr, void* pointer = nullptr);
int imap_list_all(imap_handler handler = nullptr, void *pointer = nullptr);
int imap_list_subdirs(std::string dir, imap_handler handler = nullptr, void *pointer = nullptr);
int imap_rmdir(std::string path, imap_handler handler = nullptr, void* pointer = nullptr);
int imap_mkdir(std::string path, imap_handler handler = nullptr, void* pointer = nullptr);

// open/close session
void close_curl(CURL *curl);
CURL* open_curl();

// internal command for pushing request
int _make_request(CURL *curl);

#endif /* IMAP_H */