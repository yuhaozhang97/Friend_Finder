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
#include <unordered_map>
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
#define UDP_PORT 32472
#define TCP_PORT 33472
#define SERVER_A_PORT 30472
#define SERVER_B_PORT 31472
#define MAX_SIZE 1024


// Global variables
int udp_socket_fd, tcp_socket_fd, child_tcp_socket_fd;
struct sockaddr_in server_main_TCP_addr, server_main_UDP_addr;
struct sockaddr_in client_TCP_addr, serverA_UDP_addr, serverB_UDP_addr;
char AB_send_buff[MAX_SIZE];
char AB_receive_buff[MAX_SIZE];
char client_send_buff[MAX_SIZE];
char client_receive_buff[MAX_SIZE];
unordered_map<string, string> country_map; 
string request_to_server;
string response_from_server;
string request_from_client;
string response_to_client;


void initialize_and_bind_UDP_socket() {
	// For server A and B
	// Create fd
	udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_socket_fd == -1) {
		perror("Error creating main server UDP socket.");
		exit(1);
	}

	// IP and port
	memset(&server_main_UDP_addr, 0, sizeof(server_main_UDP_addr));
	server_main_UDP_addr.sin_family = AF_INET;
	server_main_UDP_addr.sin_port = htons(UDP_PORT);
	server_main_UDP_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);

	// Bind socket
	if (bind(udp_socket_fd, (struct sockaddr*) &server_main_UDP_addr, sizeof(server_main_UDP_addr)) == -1) {
		perror("Error binding main server UDP socket.");
		exit(1);
	}
}

void set_server_A_addr() {
	// IP and port
	memset(&serverA_UDP_addr, 0, sizeof(serverA_UDP_addr));
	serverA_UDP_addr.sin_family = AF_INET;
	serverA_UDP_addr.sin_port = htons(SERVER_A_PORT);
	serverA_UDP_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
}

void set_server_B_addr() {
	// IP and port
	memset(&serverB_UDP_addr, 0, sizeof(serverB_UDP_addr));
	serverB_UDP_addr.sin_family = AF_INET;
	serverB_UDP_addr.sin_port = htons(SERVER_B_PORT);
	serverB_UDP_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
}


void initialize_and_bind_TCP_socket() {
	// For client
	// Create fd
	tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket_fd == -1) {
		perror("Error creating main server TCP socket.");
		exit(1);
	}

	// IP and port
	memset(&server_main_TCP_addr, 0, sizeof(server_main_TCP_addr));
	server_main_TCP_addr.sin_family = AF_INET;
	server_main_TCP_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
	server_main_TCP_addr.sin_port = htons(TCP_PORT);

	// Bind socket
	if (bind(tcp_socket_fd, (struct sockaddr*) &server_main_TCP_addr, sizeof(server_main_TCP_addr)) == -1) {
		perror("Error binding main serve TCP socket.");
		exit(1);
	}

	// Listen to clients
	if (listen(tcp_socket_fd, 10) == -1) {
		perror("Fail to listen to the client.");
		exit(1);
	}
}


void getCountryList() {
	sleep(1);

	// First require country list from server A
	request_to_server = "A_country_list";
	strncpy(AB_send_buff, request_to_server.c_str(), MAX_SIZE);
	set_server_A_addr();
	if (sendto(udp_socket_fd, AB_send_buff, sizeof(AB_send_buff), 0, (struct sockaddr*) &serverA_UDP_addr, sizeof(serverA_UDP_addr)) == -1) {
		perror("Fail to send country list request to server A.");
		exit(1);
	}
	// sleep(1);

	// Receive country list from A
	socklen_t A_size = sizeof(serverA_UDP_addr);
	if(recvfrom(udp_socket_fd, AB_receive_buff, MAX_SIZE, 0, (struct sockaddr *) &serverA_UDP_addr, &A_size) == -1) {
		perror("Error receiving country list from server A");
		exit(1);
	}
	char * pch_A = strtok(AB_receive_buff, " ");
	while (pch_A != NULL) {
		country_map[pch_A] = "A";
		pch_A = strtok(NULL, " ");
	}
	cout << "The Main server has received the country list from serverA using UDP over port " << UDP_PORT << endl << endl;

	
	// Second require country list from server B
	request_to_server = "B_country_list";
	strncpy(AB_send_buff, request_to_server.c_str(), MAX_SIZE);
	set_server_B_addr();
	if (sendto(udp_socket_fd, AB_send_buff, sizeof(AB_send_buff), 0, (struct sockaddr*) &serverB_UDP_addr, sizeof(serverB_UDP_addr)) == -1) {
		perror("Fail to send country list request to server B.");
		exit(1);
	}
	// sleep(1);

	// Receive country list from B
	socklen_t B_size = sizeof(serverB_UDP_addr);
	if(recvfrom(udp_socket_fd, AB_receive_buff, MAX_SIZE, 0, (struct sockaddr *) &serverB_UDP_addr, &B_size) == -1) {
		perror("Error receiving country list from server B");
		exit(1);
	}
	char * pch_B = strtok(AB_receive_buff, " ");
	while (pch_B != NULL) {
		country_map[pch_B] = "B";
		pch_B = strtok(NULL, " ");
	}
	cout << "The Main server has received the country list from serverB using UDP over port " << UDP_PORT << endl << endl;

	
	// Print out server country mapping
	cout << "Server A is responsible for:" << endl;
	for (const auto& country : country_map) {
		if (country.second == "A") {
			cout << country.first << endl;
		}
	}
	
	cout << endl << "Server B is responsible for:" << endl;
	for (const auto& country : country_map) {
		if (country.second == "B") {
			cout << country.first << endl;
		}
	}
	cout << endl;
}


int main() {
	sleep(2);
	initialize_and_bind_UDP_socket();
	initialize_and_bind_TCP_socket();
	cout << "The Main server is up and running." << endl << endl;

	getCountryList();


	while (true) {
		// Accept connection from client
		socklen_t client_addr_size = sizeof(client_TCP_addr);
		child_tcp_socket_fd = accept(tcp_socket_fd, (struct sockaddr*) &client_TCP_addr, &client_addr_size);
		if (child_tcp_socket_fd == -1) {
			perror("Fail to accept connection from client.");
			exit(1);
		}

		int num = fork();
		if (num == -1) {
			perror("Fail to create a child process handling client request.");
			exit(1);
		} else if (num == 0) {
			string country;
			string user;

			while (true) {
				if (recv(child_tcp_socket_fd, client_receive_buff, sizeof(client_receive_buff), 0) == -1) {
					perror("Could not receive request from the client.");
					exit(1);
				}

				// Process client request
				char * pch = strtok(client_receive_buff, " ");
				int i = 0;
				string msg_to_client, msg_to_server, response_from_server;
				while (pch != NULL) {
					if (i == 0) {
						country = pch;
					} else {
						user = pch;
					}
					i++;
					pch = strtok(NULL, " ");
				}

				// Check country exists or not
				if (country_map.find(country) == country_map.end()) {
				  	// Not found country
				  	cout << country << " does not show up in server A&B" << endl << endl;
				  	msg_to_client = "CountryNotFound";
				  	strncpy(client_send_buff, msg_to_client.c_str(), MAX_SIZE);
					if (send(child_tcp_socket_fd, client_send_buff, sizeof(client_send_buff), 0) == -1) {
						perror("Could not respond to client.");
						exit(1);
					}
					cout << "The Main Server has sent \"Country Name: Not found\" to client using TCP over port " << TCP_PORT << endl << endl;
				} else {
				  	// Found country
				  	cout << country << " shows up in server " << country_map[country] << endl << endl;
				  	
				  	
				  	// Send to server A/B for recommendation
				  	msg_to_server = user + " " + country;
				  	if (country_map[country] == "A") {
				  		// Send to server A
				  		strncpy(AB_send_buff, msg_to_server.c_str(), MAX_SIZE);
					  	if (sendto(udp_socket_fd, AB_send_buff, sizeof(AB_send_buff), 0, (struct sockaddr*) &serverA_UDP_addr, sizeof(serverA_UDP_addr)) == -1) {
							perror("Fail to send recommendation request to server A.");
							exit(1);
						}
						cout << "The Main Server has sent request from User " << user << " to server A using UDP over port " << UDP_PORT << endl << endl;

						// Receive from server A
						socklen_t server_size = sizeof(serverA_UDP_addr);
						if(recvfrom(udp_socket_fd, AB_receive_buff, MAX_SIZE, 0, (struct sockaddr*) &serverA_UDP_addr, &server_size) == -1) {
							perror("Error receiving recommendation from server A.");
							exit(1);
						}

				  	} else {
				  		// Send to server A
				  		strncpy(AB_send_buff, msg_to_server.c_str(), MAX_SIZE);
					  	if (sendto(udp_socket_fd, AB_send_buff, sizeof(AB_send_buff), 0, (struct sockaddr*) &serverB_UDP_addr, sizeof(serverB_UDP_addr)) == -1) {
							perror("Fail to send recommendation request to server B.");
							exit(1);
						}
						cout << "The Main Server has sent request from User " << user << " to server B using UDP over port " << UDP_PORT << endl << endl;

						// Receive from server A
						socklen_t server_size = sizeof(serverB_UDP_addr);
						if(recvfrom(udp_socket_fd, AB_receive_buff, MAX_SIZE, 0, (struct sockaddr*) &serverB_UDP_addr, &server_size) == -1) {
							perror("Error receiving recommendation from server B.");
							exit(1);
						}
				  	}


				  	// Send back to client
					response_from_server = strtok(AB_receive_buff, " ");
					if (response_from_server == "UserNotFound") {
						cout << "The Main server has received \"User ID: Not found\" from server " << country_map[country] << endl << endl;
					} else {
						cout << "The Main server has received searching result of User " << user << " from server " << country_map[country] << endl << endl;
					}

					strncpy(client_send_buff, response_from_server.c_str(), MAX_SIZE);
					if (send(child_tcp_socket_fd, client_send_buff, sizeof(client_send_buff), 0) == -1) {
						perror("Error sending result back to the client.");
						exit(1);
					}
					if (response_from_server == "UserNotFound") {
						cout << "The Main Server has sent error to client using TCP over " << TCP_PORT << endl << endl;
					} else {
						cout << "The Main Server has sent searching result to client using TCP over port " << TCP_PORT << endl << endl;
					}
					
				}
			}
		} else {
			close(child_tcp_socket_fd);
		}
	}
	

	close(udp_socket_fd);
	close(tcp_socket_fd);
	return 0;
}
