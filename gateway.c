#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <time.h>
#include "socket.h"  
#include "message.h"
#include "controller.h"

#define MAXFD(x,y) ((x) >= (y)) ? (x) : (y)

int main(int argc, char *argv[]){
	int port;
	struct cignal cig;
	// A buffer to store a serialized message
	char *cig_serialized = malloc(sizeof(char)*CIGLEN);
	// An array to registered sensor devices
	int device_record[MAXDEV] = {0};
	
	if(argc == 2){
		port = strtol(argv[1], NULL, 0);
	} else{
		fprintf(stderr, "Usage: %s port\n", argv[0]);
		exit(1);
	}

	int gatewayfd = set_up_server_socket(port);
	printf("\nThe Cignal Gateway is now started on port: %d\n\n", port);
	int peerfd;
	
	/* TODO: Implement the body of the server.  
	 *
	 * Use select so that the server process never blocks on any call except
	 * select. If no sensors connect and/or send messsages in a timespan of
	 * 5 seconds then select will return and print the message "Waiting for
	 * Sensors update..." and go back to waiting for an event.
	 * 
	 * The server will handle connections from devices, will read a message from
	 * a sensor, process the message (using process_message), write back
	 * a response message to the sensor client, and close the connection.
	 * After reading a message, your program must print the "RAW MESSAGE"
	 * message below, which shows the serialized message received from the *
	 * client.
	 * 
	 *  Print statements you must use:
     * 	printf("Waiting for Sensors update...\n");
	 * 	printf("RAW MESSAGE: %s\n", YOUR_VARIABLE);
	 */

	// TODO implement select loop
	// Suppress unused variable warning.  The next 5 ilnes can be removed 
	// after the variables are used.
    
    int max_fd = gatewayfd;
    fd_set all_fds, listen_fds;
    FD_ZERO(&all_fds);
    FD_SET(gatewayfd, &all_fds);

	while(1) {
		struct timeval tv = {5, 0};
        listen_fds = all_fds;

        int nready = select(max_fd + 1, &listen_fds, NULL, NULL, &tv);
        if (nready == -1) {
            perror("server: select");
            exit(1);
        }
        else if (nready == 0) {
            printf("Waiting for Sensors update...\n");
        }
        if (FD_ISSET(gatewayfd, &listen_fds)) { // connection
            peerfd = accept_connection(gatewayfd);
			FD_SET(peerfd, &listen_fds);
        } // after connected
		if (FD_ISSET(peerfd, &listen_fds)) {
            char buf[CIGLEN+1];
            int num_read = read(peerfd, buf, CIGLEN);
            if (num_read == -1) {
                printf("Error reading client.\n");
                exit(1);
            }
            unpack_cignal(buf, &cig);
			printf("RAW MESSAGE: %s\n", buf);
			int process=process_message(&cig, device_record);
			if (process == -1) {
				printf("Error processing message.\n");
				exit(1);
			}
            cig_serialized = serialize_cignal(cig);
            int written=write(peerfd, cig_serialized, CIGLEN);
            if (written <= 0) {
                printf("Error writing to client.\n");
                exit(1);
            }
			close(peerfd);
        }
	}
	close(gatewayfd);
	return 0;
}
