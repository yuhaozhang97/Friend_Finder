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
#include <algorithm>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fstream>

using namespace std;

// Constants
#define LOCAL_HOST "127.0.0.1"
#define UDP_PORT 30472
#define MAX_SIZE 1024


// Global variables
int serverA_socket_fd;
string request;
struct sockaddr_in serverA_addr, client_addr;
unordered_map<string, unordered_map<int, set<int> > > country_user_map;
char send_buff[MAX_SIZE];
char receive_buff[MAX_SIZE];
char receive_buff_copy[MAX_SIZE];


// Create socket fd
void initialize_socket() {
	serverA_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(serverA_socket_fd == -1){
		perror("Error creating server A socket.");
		exit(1);
	}
}

// Assign server A address and port
void initialize_sockaddr_structure() {
	memset(&serverA_addr, 0, sizeof(serverA_addr));

	serverA_addr.sin_family = AF_INET;
	serverA_addr.sin_port = htons(UDP_PORT);
	serverA_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
	// memset(&serverA_addr.sin_zero, 0, 8);
}

// Bind socket with serverA address
void bind_socket() {
	int nRet = bind(serverA_socket_fd, (struct sockaddr*) &serverA_addr, sizeof(sockaddr));
	if (nRet == -1) {
		perror("Server A failed to bind local port.");
		exit(1);
	}

	cout << "The server A is up and running using UDP on port " << UDP_PORT << endl << endl;
}

// Determine if a string is a number
bool is_number(const string& s) {
    string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

// Read data1.txt 
void fetch_country() {
	fstream newfile;
	newfile.open("data1.txt", ios::in);
	regex ws_re("\\s+");

	if (newfile.is_open()) {
		string tp;
		string cur_country;

		while (getline(newfile, tp)) {
			vector<string> elems;
			sregex_token_iterator iter(tp.begin(), tp.end(), ws_re, -1);
			sregex_token_iterator end;

			while (iter != end) {
				elems.push_back(*iter);
				iter++;
			}

			if (!is_number(elems[0])) {
				cur_country = elems[0];
			} else {
				int cur_user = stoi(elems[0]);
				if (elems.size() > 1) {
					for (int i = 1; i < elems.size(); i++) {
						country_user_map[cur_country][cur_user].insert(stoi(elems[i]));
					}
				} else {
					set<int> s;
					country_user_map[cur_country][cur_user] = s;
				}
			}
		}
		newfile.close();
	}
}

string generate_recommendation(int user, unordered_map<int, set<int> > user_map, set<int> unconnected_users) {
	int no_common_neighbor_recommendation = -1;
	int common_neighbor_recommendation = -1;
	int max_degree = 0;
	int max_common_neighbors_num = 0;
	set<int> user_neighbors = user_map[user];

	// Loop over non-neighbor(s)
	for (set<int>::iterator it = unconnected_users.begin(); it != unconnected_users.end(); ++it) {
		int cur_user = *it;
		set<int> cur_neighbors = user_map[cur_user];
		set<int> common_neighbors;

		// Find common neighbor(s)
		set_intersection(user_neighbors.begin(), user_neighbors.end(), cur_neighbors.begin(), cur_neighbors.end(), inserter(common_neighbors, common_neighbors.end()));


		if (common_neighbors.size() == 0) {
			int cur_degree = cur_neighbors.size();
			if (max_degree < cur_degree) {
				max_degree = cur_degree;
				no_common_neighbor_recommendation = cur_user;
			} else if (max_degree == cur_degree) {
				if (no_common_neighbor_recommendation == -1) {
					no_common_neighbor_recommendation = cur_user;
				} else {
					no_common_neighbor_recommendation = min(no_common_neighbor_recommendation, cur_user);
				}
			}
		} else {
			if (max_common_neighbors_num < common_neighbors.size()) {
				max_common_neighbors_num = common_neighbors.size();
				common_neighbor_recommendation = cur_user;
			} else if (max_common_neighbors_num == common_neighbors.size()) {
				if (common_neighbor_recommendation == -1) {
					common_neighbor_recommendation = cur_user;
				} else {
					common_neighbor_recommendation = min(common_neighbor_recommendation, cur_user);
				}
			}
		}
	}

	if (common_neighbor_recommendation == -1) {
		return to_string(no_common_neighbor_recommendation);
	} else {
		return to_string(common_neighbor_recommendation);
	}
}

// Print helper
void print_set() {
	for (const auto& country : country_user_map) {
        cout << "Country: " << country.first << "\n";
        for (const auto& statePair : country.second) {
            cout << "User: " << statePair.first << "\n";
            for (const auto& city : statePair.second) {
                cout << "neighbor: " << city << "\n";
            }
        }
    }
}


int main() {
	// Socket and bind
	initialize_socket();
	initialize_sockaddr_structure();
	bind_socket();

	// Initialize data
	fetch_country();

	string serverA_countries;
	for (const auto& country : country_user_map) {
		serverA_countries += country.first + " ";
	}


	while (true) {
		// Country list request
		socklen_t client_addr_size = sizeof(client_addr);
		if (recvfrom(serverA_socket_fd, receive_buff, sizeof(receive_buff), 0, (struct sockaddr*) &client_addr, &client_addr_size) == -1) {
			perror("Error receiving main server request.");
			exit(1);
		}

		strncpy(receive_buff_copy, receive_buff, MAX_SIZE);
		request = strtok(receive_buff, " ");

		// Response to the main server
		if (request == "A_country_list") {
			strncpy(send_buff, serverA_countries.c_str(), MAX_SIZE);
			if (sendto(serverA_socket_fd, send_buff, sizeof(send_buff), 0, (struct sockaddr*) &client_addr, sizeof(client_addr)) == -1) {
				perror("Failed to send country list to the main server.");
				exit(1);
			}
			cout << "The server A has sent a country list to Main Server" << endl << endl;
		} else {
			// Fetch user and country
			char * pch = strtok(receive_buff_copy, " ");
			string msg, country;
			int user;
			int i = 0;
			while (pch != NULL) {
				if (i == 0) {
					user = stoi(pch);
				} else {
					country = pch;
				}
				i++;
				pch = strtok(NULL, " ");
			}
			cout << "The server A has received request for finding possible friends of User " << user << " in " << country << endl << endl;

			// Check if user exists in country
			unordered_map<int, set<int> > user_map = country_user_map[country];
			if (user_map.find(user) == user_map.end()) {
			  	// Not found user
			  	cout << "User " << user << " does not show up in " << country << endl << endl;

			  	msg = "UserNotFound";
			  	strncpy(send_buff, msg.c_str(), MAX_SIZE);
				if (sendto(serverA_socket_fd, send_buff, sizeof(send_buff), 0, (struct sockaddr*) &client_addr, sizeof(client_addr)) == -1) {
					perror("Could not send user not found to client.");
					exit(1);
				}
				cout << "The server A has sent \"User " << user << " not found\" to Main Server" << endl << endl;
			} else {
				// Found user
				cout << "The server A is searching possible friends for User " << user << endl;

				string recommendation;
				set<int> neighbors = user_map[user];
				set<int> all_users;
				set<int> unconnected_users;

				for (unordered_map<int, set<int> >::iterator it = user_map.begin(); it != user_map.end(); ++it) {
					all_users.insert(it->first);
				}
				all_users.erase(user);

				// all_users - neighbors
				set_difference(all_users.begin(), all_users.end(), neighbors.begin(), neighbors.end(), inserter(unconnected_users, unconnected_users.end()));

				if (unconnected_users.size() == 0) {
					recommendation = "None";
				} else {
					recommendation = generate_recommendation(user, user_map, unconnected_users);
				}

				// Send back main server
				cout << "Here are the results: User " << recommendation << endl << endl;
				strncpy(send_buff, recommendation.c_str(), MAX_SIZE);
				if (sendto(serverA_socket_fd, send_buff, sizeof(send_buff), 0, (struct sockaddr*) &client_addr, sizeof(client_addr)) == -1) {
					perror("Could not send recommendation to client.");
					exit(1);
				}
				cout << "The server A has sent the result to Main Server" << endl << endl;
			}
		}
	}


	close(serverA_socket_fd);
	return 0;
}
