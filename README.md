##Problems Solved##
The project implements a friend recommendation system that aims to provide the users friend with whom they could potentially connect. The users can simply input their user ID and country and wait for the result recommended by the system.

##Source Files##
There are a total of 4 files included. serverA.cpp stores the user-neighbor map of specific countries, and generate friend recommendation; Similarly, serverB stores user-neighbor map of the other countries (no overlapping country between server A and B); client.cpp allows the users to input their user ID and country name, and send them to the main server for recommendation request; servermain.cpp acts as an intermediate between the client and server A/B. It receives the client request and forward the request to server A or B. After getting back from A/B, it sends the result back to the client. Note that if the input country does not exist, the main server will directly reply the client with a "country not found" message without sending the request to A/B.

##Some important messages sent between the entites##
###server A/B:###
* Boot up: The server A/B is up and running using UDP 30472/31472.
* Upon receiving recommendation request: The server A/B received request for finding possible friends of User <ID> in <country name>.
* Find recommendation: The server A has sent the result to Main Server.

###servermain:###
* Boot up: The Main server is up and running.
* Upon receiving input from the client: The Main server has received the request of User <ID> in <country name> from client using TCP over port 33472.
* Send request to A/B: The Main server has sent request from User <ID> to server A/B using UDP over port 32472.
* Upon receiving result from A/B: The Main server has received searching result of User <ID> from server A/B.
* Send result back to client: The Main Server has sent searching result to client using TCP over port 33472.

###client:###
* Boot up: Client is up and running.
* After sending User ID to Main Server: Client has sent User <ID> and <country name> to Main Server using TCP.
* Upon Receiving recommendation from Main Server: Client has received result from Main Server, Recommended User ID is <ID>.

##Idiosyncrasy##
This project uses a nested map (rather than adjacency matrix) to store country - user - neighbor relationship. This system will not work if the source data1.txt and data2.txt are modified while the system is running (need to reboot all servers to process the new information added/deleted).
