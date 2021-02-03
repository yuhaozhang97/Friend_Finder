#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <sstream>
#include <set>
#include <map>
#include <regex>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fstream>

using namespace std;

// Constants
#define LOCAL_HOST "127.0.0.1"
#define MAX_SIZE 1024
#define SERVER_PORT 33472


// Global variables
int socket_fd;
char receive_buff[MAX_SIZE];
char send_buff[MAX_SIZE];
struct sockaddr_in server_addr;
string input_country, input_user, response;


void initialize_and_connect_server() {
	// Create fd
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1) {
		perror("Error opening client socket.");
		exit(1);
	}

	// IP and port
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
	server_addr.sin_port = htons(SERVER_PORT);

	cout << "The client is up and running" << endl << endl;
}

void send_connect_request() {
	if (connect(socket_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1) {
		perror("Connection failed.");
		close(socket_fd);
		exit(1);
	}
}

bool is_number(const string& s) {
    string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}


int main() {
	initialize_and_connect_server();
	send_connect_request();


	while (true) {
		cout << "-----Start a new request-----" << endl;
		cout << "Please enter country name: ";
		getline(cin, input_country);
		cout << "Please enter user ID: ";
		getline(cin, input_user);
		cout << endl;

		if (!is_number(input_user)) {
			cout << "Please enter user ID again (needs to be an int): ";
			getline(cin, input_user);
			cout << endl;
		}


		// Send to the main server
		string send_msg = input_country + " " + input_user;
		strncpy(send_buff, send_msg.c_str(), MAX_SIZE);
		if (send(socket_fd, send_buff, sizeof(send_buff), 0) == -1) {
			perror("Failed to send data to the server.");
			close(socket_fd);
			exit(1);
		}
		cout << "Client has sent User " << input_user << " and Country " << input_country << " to Main Server using TCP" << endl << endl;

		// Receive from the main server
		if (recv(socket_fd, receive_buff, sizeof(receive_buff), 0) == -1) {
			perror("Failed to receive recommendation result from the server.");
			close(socket_fd);
			exit(1);
		}
		
		response = strtok(receive_buff, " ");
		
		if (response == "CountryNotFound") {
			cout << input_country << " not found" << endl << endl;
		} else if (response == "UserNotFound") {
			cout << "User " << input_user << " not found" << endl << endl;
		} else {
			cout << "Client has received result from Main Server: " << endl;
			cout << "Recommended User ID is " << response << endl << endl;
		}
	}


	close(socket_fd);
	return 0;
}
