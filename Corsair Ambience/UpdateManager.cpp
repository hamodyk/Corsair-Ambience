#include "UpdateManager.h"
#include <curl/curl.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include "stdafx.h"


using json = nlohmann::json;

size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s) {
	size_t newLength = size * nmemb;
	size_t oldLength = s->size();
	try {
		s->resize(oldLength + newLength);
	}
	catch (std::bad_alloc &e) {
		//handle memory problem
		std::cout << "Error: couldn't allocate enough memory for the HTTP response" << std::endl;
		std::cout << e.what() << std::endl;
		return 0;
	}

	std::copy((char*)contents, (char*)contents + newLength, s->begin() + oldLength);
	return size * nmemb;
}

void UpdateManager::checkForAnUpdate() {
	try {
		CURL *curl;
		CURLcode res;
		std::string response;
		curl_global_init(CURL_GLOBAL_DEFAULT);
		struct curl_slist *list = NULL;
		curl = curl_easy_init();
		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/repos/hamodyk/Corsair-Ambience/releases/latest");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
			list = curl_slist_append(list, "User-Agent: C/1.0");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

			#ifdef SKIP_PEER_VERIFICATION
			/*
			* If you want to connect to a site who isn't using a certificate that is
			* signed by one of the certs in the CA bundle you have, you can skip the
			* verification of the server's certificate. This makes the connection
			* A LOT LESS SECURE.
			*
			* If you have a CA cert for the server stored someplace else than in the
			* default bundle, then the CURLOPT_CAPATH option might come handy for
			* you.
			*/
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			#endif

			#ifdef SKIP_HOSTNAME_VERIFICATION
			/*
			* If the site you're connecting to uses a different host name that what
			* they have mentioned in their server certificate's commonName (or
			* subjectAltName) fields, libcurl will refuse to connect. You can skip
			* this check, but this will make the connection less secure.
			*/
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
			#endif

			/* Perform the request, res will get the return code */
			res = curl_easy_perform(curl);
			/* Check for errors */
			if (res != CURLE_OK) {
				//fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
				std::cout << "Error: Unable to retrieve information about the latest version from Github" << std::endl;
				std::cout << curl_easy_strerror(res) << "\n" << std::endl;
				return;
			}

			/* always cleanup */
			curl_easy_cleanup(curl);
			curl_slist_free_all(list);
		}

		curl_global_cleanup();

		auto js = json::parse(response);
		std::string latestVersion = js["tag_name"];
		std::string currentVersion = version;
		if (latestVersion.compare(currentVersion) != 0) { //if they are not equal
			std::cout << "a new version (" << latestVersion << ") is available to download!" << std::endl;
			std::cout << "Visit " << "https://github.com/hamodyk/Corsair-Ambience" << " for more info" << std::endl;
		}
	}
	catch (const std::exception& e) {
		std::cout << "Error: Unable to retrieve information about the latest version from Github" << std::endl;
		std::cout << e.what() << "\n" << std::endl;
	}
}