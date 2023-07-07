#ifndef _REQUESTS_
#define _REQUESTS_

#include <vector>

char *compute_get_request(char *host, char *url, string query_params,
							vector<string> cookies, int cookies_count, string authorization);


char *compute_post_request(char *host, char *url,
                           char *content_type, string body_data,
                           int body_data_fields_count, vector<string> cookies,
                           int cookies_count, string authorization);

char *compute_delete_request(char *host, char *url, string query_params,
                            vector<string> cookies, int cookies_count, string authorization);

#endif
