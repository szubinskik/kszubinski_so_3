#include "imap.h"

const char *login, *passwd;
struct curl_slist *host = NULL;

void init_curl(const char* login, const char* passwd)
{
	::login = login;
	::passwd = passwd;
	host = curl_slist_append(NULL, "imap.gmail.com:993:173.194.220.108");
	// TODO ?
}

CURL* open_curl()
{
	CURL* curl = curl_easy_init();

	// Set username and password
	curl_easy_setopt(curl, CURLOPT_USERNAME, login);
	curl_easy_setopt(curl, CURLOPT_PASSWORD, passwd);

	// Force DNS resolving - fix local problems
	// TODO
	curl_easy_setopt(curl, CURLOPT_RESOLVE, host);

	// This is just the GMail server URL
	curl_easy_setopt(curl, CURLOPT_URL, "imaps://imap.gmail.com:993/");
	return curl;
}

void close_curl(CURL *curl)
{
	curl_easy_cleanup(curl);
}

int _make_request(CURL *curl)
{
	CURLcode res = CURLE_OK;

	if (curl)
	{
		// Perform the fetch
		res = curl_easy_perform(curl);
	}

	return (int)res;
}