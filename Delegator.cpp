// Sockets Reference:
// http://linux.die.net/man/2/socket
// http://linux.die.net/man/2/accept
// http://linux.die.net/man/2/listen
#include "delegator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <vector>
#include <algorithm> // std::reverse, can be ditched soon.
#include <string>
#include <iostream>

std::vector<int> connections;

Delegator::Delegator(){
  puts("New delegator created.  Perhaps a rename is in order?");
}

void Delegator::error(const char *msg){
  perror(msg);
  exit(1);
}

void Delegator::spawn_tcp_listener() {
  pthread_t thread_id;
  int i = 42;
  if( pthread_create( &thread_id, NULL, tcp_listener,  (void *)&i) < 0){
      perror("could not create thread");
  }
}

void Delegator::stop_listener() {
  listening = false;
}

// This function should be passed to another thread, also
void *Delegator::tcp_listener(void *i) {
  int sock_fd, client_sock, c;
  struct sockaddr_in server, client;

  // Socket File Descriptor
  sock_fd = socket(AF_INET , SOCK_STREAM , 0);

  if (sock_fd == -1){
    printf("Could not create socket");
  }

  server.sin_family = AF_INET; // AF_INET == IPv4 Internet protocols.  We could be cool and go IPv6...
  server.sin_addr.s_addr = INADDR_ANY; // server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_port = htons( 8888 );

  // Bind
  if (bind(sock_fd,(struct sockaddr *)&server , sizeof(server)) < 0){
    perror("bind failed. Error");
  }

  puts("bind done");

  // Mark socket as passive, ready to receive
  listen(sock_fd , 3);

  //Accept incoming connections
  puts("Waiting for incoming connections...");
  c = sizeof(struct sockaddr_in);
  pthread_t thread_id;

  while( (client_sock = accept(sock_fd, (struct sockaddr *)&client, (socklen_t*)&c)) ){
      puts("Connection accepted");

      if( pthread_create( &thread_id, NULL,  connection_handler, (void*) &client_sock) < 0){
          perror("could not create thread");
      }

      puts("Handler assigned");
  }

  if (client_sock < 0){
      perror("accept failed");
  }
}

void *Delegator::connection_handler(void *socket_desc) {
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message, client_message[90000];
    connections.push_back(sock);

    //Send some messages to the client
    message = "Greetings! I am your connection handler\n";
    write(sock , message , strlen(message));

    message = "Now type something and i shall reverse it \n";
    write(sock , message , strlen(message));

    //Receive a message from client
    while( (read_size = recv(sock , client_message , 90000 , 0)) > 0){
        //end of string marker
        client_message[read_size] = '\0';

        //Send the message back to client
        std::reverse( client_message, &client_message[ strlen( client_message ) ] );
        write(sock , client_message , strlen(client_message));

        //clear the message buffer
        memset(client_message, 0, 90000);
    }

    if(read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if (read_size == -1) {
        perror("recv failed");
    }

    return 0;
}

void Delegator::tcp_image_poll(){
  char *message = "CurrFrame";
  for (int sock : connections ){
    write(sock, message , std::strlen(message));
  }
}

void Delegator::connection_check(){
  int s = connections.size();
  std::string rs = std::to_string(s);
  std::string msg = "There are " + rs + " connections in the connection pool";
  std::cout << msg;
}

bool Delegator::is_listening(){
  return listening;
}
