Copyright: Dobrica Nicoleta-Adriana 321CAa

# Web Client with REST API


## Description:

- This homework implements a web client that interacts with a REST API.
Specifically, it shows the interaction between a client and a server, where the client sends different commands to the server, and the server responds accordingly.

- For manipulating the JSON objects, I used the nlohmann C++ library, because it provides a clear and simple interface for JSON, and it was easier to use an already existing library for parsing JSON objects than doing it manually.


## Implementation:


- **Register command**:

    - If the user wants to register, then the username and the password will be 
    prompted to the user, and the request to register will then be sent to the 
    server as a POST request. The response from the server is then parsed, to
    see if an error popped up or not: the user might get a bad request, if the
    prompted username already exists, or no errors, meaning that the user managed
    to register successfully.

- **Login command**:

    - If a user wants to log in, then we will create a JSON payload with the
    prompted username and password, and the client will send a POST request to
    the server for authenticating. 

    - There are different errors that the server can send to the client; if
    the user prompted credentials that match no username, or if the username
    does exist, but the password is incorrect. 

    - If no errors show up, then the user managed to log in succesfully, and 
    the response from the server contains a session cookie to be used later.

- **Enter library command**:

    - The user can access the library, but they need to first be logged in.
    The session cookie returned earlier by the server when the user authenticated
    allows the client to get acess to the library, by sending a GET request to
    the server. The server sends the client a JWT token, that will alow the client
    later to see books from the library, or to add or delete them.

- **Get books command**:
    - This command sends a GET request to the server that prints in JSON format
    all of the user's available books, in a specific format: the books' id, title,
    author and other info. If the user does not have a JWT cookie that proves they
    have access to the library, then the server will send an error message back.
    Otherwise, the server will send the user their requested books.

- **Add book command**:

    - With this command, the client can add a book to the library. If the given
    inputs are invalid (e.g. the page count is not a valid number), then an error
    will pop up, and the client needs to write the add books command again in
    order to add a book with valid input. If everything is correct, then a JSON
    object will be created and the client will compute a POST request to the
    server. If the user does not have access to the library, then the server will
    send back an error, otherwise the book will be added to the library.

- **Get book command**:

    - Instead of sending back all of the user's books, the server will send just
    one book back to the client, with the given id. If the id is not valid, then
    the user will be prompted to input a valid id. Then, a GET request will be
    sent to the server. There are two errors that the server can send back to 
    the client: if either there is currently no book with the given id in the
    library, or if the user does not have access to the library. Otherwise, the
    server will send the client the requested book, parsed in JSON format.

- **Delete book command**:

    - The opposite of the add book command, if the client wants to delete a certain
    book with a given id, then the client will send a DELETE request to the server. 
    There are two errors that the server can send back to the client: if no book
    with the given id was deleted, or if the client does not have access to the
    library.

- **Logout command**:
    - This command logs the user, deleting their session cookie and their library
    access token (JWT Token).

- **Exit command**:
    - Stops the program
