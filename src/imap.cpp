#include "imap.h"

const char *login, *passwd, *server;
struct curl_slist *host = NULL;

void init_curl(const char *login, const char *passwd, const char *server, const char *resolve)
{
	::login = login;
	::passwd = passwd;
	::server = server;

	if (resolve != nullptr)
		host = curl_slist_append(NULL, resolve);
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

	curl_easy_setopt(curl, CURLOPT_URL, server);
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