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
#include <string>
#include <opencv2/opencv.hpp>
#include <time.h> // potentially replaced by mutex
#include <mutex>

std::vector<int> connections;
std::mutex mut;
int image_count = 0;
long image_id = 0;

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
  } else {
    printf("Delegator is listening");
  }

  server.sin_family = AF_INET; // AF_INET == IPv4 Internet protocols.  We could be cool and go IPv6...
  server.sin_addr.s_addr = INADDR_ANY; // server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_port = htons( 8888 );

  // Bind
  if (bind(sock_fd,(struct sockaddr *)&server , sizeof(server)) < 0){
    perror("bind failed. Error");
  }

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
  // Get the socket descriptor
  int sock = *(int*)socket_desc;
  int read_size;
  char *message, client_message[1000];
  long byteStreamLimit;
  bool expectingImage = false; // TODO: This will change with Protocol Buffers
  connections.push_back(sock);

  // Receive messages from client
  while(true){
    if (expectingImage){
      // Go back to listening for image size.
      expectingImage = false;

      // Allocate now that we know exactly how big buffer should be
      unsigned char imageBytes[byteStreamLimit];
      std::cout << "reading byte stream of char * [" << byteStreamLimit << "]" << std::endl;

      // wait to read in to buffer, stop reading once we hit buffer size
      read_size = recv(sock , imageBytes , byteStreamLimit , 0);

      // Handle read failure
      if(read_size == 0) { // TODO: DRY this up.  Put it in a function and call it here and in previous if block
          puts("Client disconnected");
          fflush(stdout);
          break;
      } else if (read_size == -1) {
          perror("recv failed");
          break;
      }

      // Build OpenCV img object in memory. Mat can take a byte vector as a constructor.
      std::vector<unsigned char> byteVec(imageBytes, imageBytes + byteStreamLimit);
      cv::Mat data_mat(byteVec,true);
      cv::Mat frame(cv::imdecode(data_mat,1));

      // Save img object in memory to disk
      try {
        std::string rel_path = "stitched_images/stitchedImage" + std::to_string(sock) + ".png";
        cv::imwrite(rel_path, frame);
      } catch (std::runtime_error& ex) {
        fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
      }

      // clear buffer
      memset(imageBytes, 0, byteStreamLimit);

    } else {
      // We're parsing a small NULL-terminated string.  Buffer can be arbitrary small-ish size
      read_size = recv(sock , client_message , 1000 , 0);
      // Handle read failure
      if(read_size == 0) { // TODO: DRY this up.  Put it in a function and call it here and in previous if block
          puts("Client disconnected");
          fflush(stdout);
          break;
      } else if (read_size == -1) {
          perror("recv failed");
          break;
      }

      // Handle generic messages
      if(!strstr(client_message, "SendingFrame")){ // should probably implement a message library to handle strings
        perror("Couldn't parse: strstr  (163)");
        write(sock , "couldn't parse" , 14);
        continue;
      }

      std::cout << "client: " << client_message << std::endl;

      // Parse bufferlength from client
      std::string msg(client_message);
      byteStreamLimit = std::stol(msg.substr(msg.find(",") + 1));
      expectingImage = true;
      std::cout << "waiting for image of size " << byteStreamLimit << std::endl;

      //clear the message buffer
      memset(client_message, 0, 1000);
    }
  }
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
