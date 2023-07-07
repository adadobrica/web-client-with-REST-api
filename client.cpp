#include <stdio.h>     
#include <stdlib.h>    
#include <unistd.h>     
#include <string.h>     
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>      
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include <iostream>
#include "nlohmann/json.hpp"
#include <string>
#include <map>
#include <vector>

#define PORT 8080

using namespace std;

using json = nlohmann::json;

char ip_addr[] = "34.254.242.81";
char ip_and_port[] = "34.254.242.81:8080";
char api_addr[] = "/api/v1/tema/auth/register";
char payload_type[] = "application/json";
char api_login[] = "/api/v1/tema/auth/login";
char library_access[] = "/api/v1/tema/library/access";
char get_books_api[] = "/api/v1/tema/library/books";

string session_cookie;
string JWT_token;

/* function that registers the user */

void register_user(string username, string password) {
    int sockfd = open_connection(ip_addr, PORT, AF_INET, SOCK_STREAM, 0);

    char* request;
    char* response;

    /* creating our json object */

    json payload;
    payload["username"] = username;
    payload["password"] = password;

    /* formatting our json object as a string */

    string payload_data = payload.dump();

    vector<string> cookies;

    /* sending the server a post request */

    request = compute_post_request(ip_and_port, api_addr, payload_type, payload_data, 1, cookies, 0, "");

    send_to_server(sockfd, request);

    response = receive_from_server(sockfd);

    /* parse response from server */

    const char* error_msg = "error";

    char *result = strstr(response, error_msg);

    if (result != nullptr) {
        cout << "Bad request: username already exists!\n";
    } else {
        cout << "New user created!\n";
    }

    close(sockfd);
}

/* function that parses the response from the server and returns a session cookie */

string get_session_cookie(char *response) {
    string http_response(response);
    string set_cookie = "Set-Cookie: ";

    auto str_it = http_response.find(set_cookie);

    auto str_start_pos = str_it + set_cookie.length();
    auto str_end_pos = http_response.find_first_of(";", str_start_pos);

    string cookie = http_response.substr(str_start_pos, str_end_pos - str_start_pos);

    return cookie;
}

/* function used when the user logs in */

void auth_user(string username, string password) {
    int sockfd = open_connection(ip_addr, PORT, AF_INET, SOCK_STREAM, 0);

    char *request, *response;

    /* creating a json payload for authenticating */

    json login;
    login["username"] = username;
    login["password"] = password;

    string login_payload = login.dump();

    vector<string> cookies;

    request = compute_post_request(ip_and_port, api_login, payload_type, login_payload, 1, cookies, 0, "");

    send_to_server(sockfd, request);

    response = receive_from_server(sockfd);

    /* parse response from server */

    const char* error_msg = "Credentials are not good!";

    char *result = strstr(response, error_msg);

    if (!result) {
        /* if the server did not send a "wrong credentials" error, then we search
        for the other type of error we can encounter */

        const char *no_account = "No account with this username!";

        char *tmp = strstr(response, no_account);

        if (!tmp) {
            cout << "User authenticated successfully.\n";

            /* get session cookie */

            session_cookie = get_session_cookie(response);
        } else {
            cout << "Error: No account with this username!\n";
        }

    } else {
        cout << "Error: Credentials are not good!\n";
    }

    close(sockfd);
}

/* function used to parse the JWT token from the server */

string get_JWT_token(char *response) {
    string library_response(response);

    string token = "token\":\"";

    auto start_pos = library_response.find(token) + token.length();
    auto end_pos = library_response.find("\"}");

    /* adding "Bearer" to the front of the string */

    string jwt = "Bearer ";
    string s = library_response.substr(start_pos, end_pos - start_pos);
    
    jwt += s;

    return jwt;

}

/* function that parses the response from the server when the user wants to enter
the library and checks whether the server sent errors back */

void parse_library_access_response(char *response) {
    /* check if the response contains an error */

    const char *error_msg = "You are not logged in!";
    char *res = strstr(response, error_msg);

    if (!res) {
        /* we don't have an error, so we get the JWT token */

        JWT_token = get_JWT_token(response);

        cout << "Entered library successfully!\n";

    } else {
        cout << "Error: You are not logged in!\n";
    }

}

/* function used if the user wants to enter the library and returns a JWT token
    if successful */

void enter_library() {
    int sockfd = open_connection(ip_addr, PORT, AF_INET, SOCK_STREAM, 0);

    char *request, *response;

    /* adding the session cookie when computing our get request so we get access
        to the library */

    vector<string> cookies;
    cookies.push_back(session_cookie);

    request = compute_get_request(ip_and_port, library_access, "", cookies, 1, "");

    send_to_server(sockfd, request);

    response = receive_from_server(sockfd);

    /* parse response from server */

    parse_library_access_response(response);

    close(sockfd);

}

/* function that parses the response from the server when the client wants to
    use the get books command  */

void parse_get_books_response(char *response, bool &error) {
    const char* error_msg = "Authorization header is missing!";

    char *res = strstr(response, error_msg);

    if (!res) {
        /* no errors were found */
        return;
    } else {
        cout << "Error: Unauthorized access!" << "\n";
        error = true;
    }
}

/* function that parses the json returned by the server */

void parse_json_books_array(char *response) {
    char *find_json = strchr(response, '[');

    string obj(find_json);

    json books = json::parse(obj);

    cout << books.dump(4) << "\n";
}

/* function used when the user wants to see their books, sends a get request to the server */

void get_books() {
    int sockfd = open_connection(ip_addr, PORT, AF_INET, SOCK_STREAM, 0);

    char *request, *response;

    vector<string> cookies;

    request = compute_get_request(ip_and_port, get_books_api, "", cookies, 0, JWT_token);

    send_to_server(sockfd, request);

    response = receive_from_server(sockfd);

    bool error = false;

    parse_get_books_response(response, error);

    /* if no errors were encountered, then we parse the books the server sent */

    if (error == false) {
        parse_json_books_array(response);
    }

    close(sockfd);
}

/* function that parses the response from the server when the client wants
    to add a book to its library */

void parse_add_book_response(char *response) {
    string str(response);

    const char* error_msg = "Authorization header is missing!";
    char *res = strstr(response, error_msg);

    if (!res) {
        cout << "Book added successfully!\n";
    } else {
        cout << "Error: User does not have access to library!\n";
    }

}

/* function that checks the integrity of the arguments given when the user wants to
    add a new book to the library */

int check_args_formatting(string title, string author, string genre, string publisher, string page_count) {
    if (title.length() == 0 || author.length() == 0 || genre.length() == 0
        || publisher.length() == 0 || page_count.length() == 0) {
        return -1;
    }

    /* also check if page_count is a number */

    try {
        int num = stoi(page_count);
        if (num >= 0) {
            return 0;
        }
        if (num < 0) {
            return -1;
        }

    } catch (const std::invalid_argument& e) {
        return -1;
    }

    return 0;
}

/* function that computes a post request and sends it to the server if the user 
    wants to add a new book */

void add_book(string title, string author, string genre, string publisher, string page_count) {
    int sockfd = open_connection(ip_addr, PORT, AF_INET, SOCK_STREAM, 0);

    int ok = check_args_formatting(title, author, genre, publisher, page_count);

    /* if the arguments are wrong, then we skip the rest, and the user will need to prompt
    the add book command again in order to try to add a new book */

    if (ok == -1) {
        cout << "Wrong format for arguments!\n";
        return;
    }

    char *request, *response;

    /* creating a new JSON object based on the given arguments */

    json json_book;

    vector<string> cookies;

    json_book["title"] = title;
    json_book["author"] = author;
    json_book["genre"] = genre;
    json_book["page_count"] = page_count;
    json_book["publisher"] = publisher;

    string new_book = json_book.dump();

    request = compute_post_request(ip_and_port, get_books_api, payload_type, new_book, 1, cookies, 0, JWT_token);

    send_to_server(sockfd, request);

    response = receive_from_server(sockfd);

    /* parse response from server */

    parse_add_book_response(response);

    close(sockfd);

}

/* function that parses the response from the server if the user wants to get a book 
    with a certain id */

void parse_book_id_response(char *response, bool &error, string id) {

    /* check if there is a book with the given id in the library, 
        or if the user has access to the library */

    const char* error_msg = "No book was found!";

    char* res = strstr(response, error_msg);

    if (!res) {
        const char* auth_error = "Authorization header is missing!";
        char* ans = strstr(response, auth_error);

        if (!ans) {
            return;
        } else {
            cout << "Error: user not authorized!\n";
            error = true;
        }
    } else {
        cout << "404 Not Found: No book with id " << id << " was found!\n";
        error = true;
    }
}

/* function that parses the JSON book sent by the server */

void parse_json_book(char *response, string id) {

    char *find_json = strchr(response, '{');

    string obj(find_json);

    json j = json::parse(obj);

    json book;
    book["id"] = id;
    book["title"] = j["title"];
    book["author"] = j["author"];
    book["publisher"] = j["publisher"];
    book["genre"] = j["genre"];
    book["page_count"] = j["page_count"];

    cout << book.dump(4) << "\n";

}

/* function similar to get books, but only returns a specific book*/

void get_book_with_id(string id) {
    int sockfd = open_connection(ip_addr, PORT, AF_INET, SOCK_STREAM, 0);

    char api_books_id[] = "/api/v1/tema/library/books/";
    strcat(api_books_id, id.c_str());

    char *request, *response;

    vector<string> cookies;

    request = compute_get_request(ip_and_port, api_books_id, "", cookies, 0, JWT_token);

    send_to_server(sockfd, request);

    response = receive_from_server(sockfd);

    bool error = false;

    parse_book_id_response(response, error, id);

    if (error == false) {
        parse_json_book(response, id);
    }

    close(sockfd);
}

/* parsing the response from the server if the client wants to delete a book, 
    checking if the user has access to the library or if there wasn't a book
    with the given id that was deleted */

void parse_delete_book_response(char *response, string id) {

    const char* error_msg = "No book was deleted!";

    char* res = strstr(response, error_msg);

    if (!res) {
        const char* auth_error = "Authorization header is missing!";
        char* ans = strstr(response, auth_error);

        if (!ans) {
            cout << "Book deleted successfully!\n";
            return;
        } else {
            cout << "Error: user not authorized!\n";
        }
    } else {
        cout << "404 Not Found: No book with id "<< id << " was deleted!\n";
    }
}

/* sends a delete request to the server in order to delete a specific book */

void delete_book(string id) {
    int sockfd = open_connection(ip_addr, PORT, AF_INET, SOCK_STREAM, 0);

    char api_books_id[] = "/api/v1/tema/library/books/";
    strcat(api_books_id, id.c_str());

    char *request, *response;

    vector<string> cookies;

    request = compute_delete_request(ip_and_port, api_books_id, "", cookies, 0, JWT_token);

    send_to_server(sockfd, request);

    response = receive_from_server(sockfd);

    parse_delete_book_response(response, id);

    close(sockfd);

}

/* checking if the given id is valid */

int check_id(string id) {
    try {
        int num = stoi(id);
        if (num >= 0) {
            return 0;
        } else {
            return -1;
        }

    } catch (const std::invalid_argument& e) {
        return -1;
    }
}


int main(int argc, char *argv[]) {
    string input;
    string username, password;

    while (1) {
        getline(cin, input);

        if (input == "register") {
            /* the user wants to register */

            cout << "username=";

            getline(cin, username);

            cout << "password=";

            getline(cin, password);

            register_user(username, password);

        } else if (input == "exit") {
            break;
        } else if (input == "login") {
            cout << "username=";

            getline(cin, username);

            cout << "password=";

            getline(cin, password);

            auth_user(username, password);

        } else if (input == "enter_library") {
            enter_library();

        } else if (input == "get_books") {
            get_books();

        } else if (input == "get_book") {
            string id;
            cout << "id=";

            getline(cin, id);

            if (check_id(id) == 0) {
                get_book_with_id(id);
            } else {
                cout << "Invalid id! Please enter a number.\n";
            }

        } else if (input == "add_book") {
            string title, author, publisher, genre;
            string page_count;

            cout << "title=";
            getline(cin, title);

            cout << "author=";
            getline(cin, author);

            cout <<"genre=";
            getline(cin, genre);

            cout << "publisher=";
            getline(cin, publisher);

            cout<<"page_count=";
            getline(cin, page_count);

            add_book(title, author, genre, publisher, page_count);

        } else if (input == "delete_book") {
            string id;

            cout << "id=";
            getline(cin, id);

            delete_book(id);

        } else if (input == "logout") {
            JWT_token = "";
            session_cookie = "";

            cout << "User logged out successfully!\n";

        } else {
            cout << "Invalid command!\n";
        }
    }


    return 0;
}
