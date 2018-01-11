#include "imap.h"

char *login, *passwd;
CURL *curl;

void init_curl(char* login, char* passwd)
{
	curl = curl_easy_init();
	::login = login;
	::passwd = passwd;
	// TODO ?
}

int _make_request()
{
	CURLcode res = CURLE_OK;

	if (curl)
	{
		// Set username and password
		curl_easy_setopt(curl, CURLOPT_USERNAME, login);
		curl_easy_setopt(curl, CURLOPT_PASSWORD, passwd);

		// Force DNS resolving - fix local problems
		// TODO
		struct curl_slist *host = NULL;
		host = curl_slist_append(NULL, "imap.gmail.com:993:173.194.220.108");
		curl_easy_setopt(curl, CURLOPT_RESOLVE, host);

		// This is just the GMail server URL
		curl_easy_setopt(curl, CURLOPT_URL, "imaps://imap.gmail.com:993/");

		// Perform the fetch
		res = curl_easy_perform(curl);

		// Check for errors
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

		// Always cleanup
		curl_easy_cleanup(curl);
		curl = curl_easy_init();
	}

	return (int)res;
}