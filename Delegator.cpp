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
// std::mutex mut;
int image_count = 0;
long image_id = 0;

Delegator::Delegator(){
  puts("New delegator created.  Perhaps a rename is in order?");
}

void Delegator::error(const char *msg){
  perror(msg);
  exit(1);
}

std::string Delegator::thread_safe_file_name(std::string prefix, std::string extension){
    std::string name = prefix + std::to_string(image_id) + extension;
    image_id++;
    image_count++; // TODO: this should be called after the try block in the image save, but it needs a mutex
    return name;
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
  // Get the socket descriptor
  int sock = *(int*)socket_desc;
  int read_size;
  char *message, client_message[1000];
  long byteStreamLimit;
  bool expectingImage = false; // TODO: This will change with Protocol Buffers
  connections.push_back(sock);

  //Send some messages to the client
  // message = "Greetings! I am your connection handler\n";
  // write(sock , message , strlen(message));
  //
  // message = "Now type something and i shall reverse it \n";
  // write(sock , message , strlen(message));

  //Receive messages from client
  while( true ){
    puts("while ran");
    if (expectingImage){
      puts("Now we're expecting an image");
      expectingImage = false;
      unsigned char imageBytes[byteStreamLimit];
      puts("Array declared");
      std::cout << "we will read size of " << byteStreamLimit << std::endl;
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
      puts("time to convert!");
      // Char array to vec
      std::vector<unsigned char> byteVec(imageBytes, imageBytes + byteStreamLimit);

      cv::Mat data_mat(byteVec,true);
      cv::Mat frame(cv::imdecode(data_mat,1));

      // int iRand;

      // srand(time(NULL));
      // iRand = rand() % 100000;

      // build image path/name
      std::string filenameString = "stitchedImage" + std::to_string(sock) + ".png";
      std::string rel_path = "stitched_images/" + filenameString;
      std::cout << "The file path is::: " << rel_path << std::endl;
      try {
        cv::imwrite(rel_path, frame);
      } catch (std::runtime_error& ex) {
        fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
      }

      // clear buffer
      memset(imageBytes, 0, byteStreamLimit);
      if (image_count == connections.size()) {
        /* stitch images */
      }

    } else {
      read_size = recv(sock , client_message , 1000 , 0);
      puts("We're expecting a normal message");
      // Handle read failure
      if(read_size == 0) { // TODO: DRY this up.  Put it in a function and call it here and in previous if block
          puts("Client disconnected");
          fflush(stdout);
          break;
      } else if (read_size == -1) {
          perror("recv failed");
          break;
      }
      std::cout << "the client message is: " << client_message << std::endl;
      // Handle generic messages
      if(!strstr(client_message, "SendingFrame")){ // should probably implement a message library to handle strings
        perror("Couldn't parse: strstr  (163)");
        write(sock , "couldn't parse" , 14);
        continue;
      }

      // Parse bufferlength from client. e.g. py call: "SendingFrame,%d\n"
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
