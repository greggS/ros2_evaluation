#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <time.h>			
#include <unistd.h>			
#include <sys/mman.h>		
#include <sched.h>			
#include <arpa/inet.h>  // socket

#define EVAL_NUM 120
#define IS_RELIABLE_QOS 0			// 1 means "reliable"", 0 means "best effort""


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

static const rmw_qos_profile_t rmw_qos_profile_reliable = {
  RMW_QOS_POLICY_HISTORY_KEEP_ALL,
  5,
  RMW_QOS_POLICY_RELIABILITY_RELIABLE,
  RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL
};

static const rmw_qos_profile_t rmw_qos_profile_best_effort = {
  RMW_QOS_POLICY_HISTORY_KEEP_LAST,
  1,
  RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT,
  RMW_QOS_POLICY_DURABILITY_VOLATILE
};

int i, initializer=0;

struct timespec tp1;		
FILE *fp;				

struct sockaddr_in addr;		// for socket
struct sockaddr_in client;
socklen_t client_size;
int sock0, sock;

void chatterCallback(const std_msgs::msg::String::SharedPtr msg) {

  std::string termination = msg->data.c_str();
  if (termination.find("end") != std::string::npos) {
	  printf("---end evaluation---\n");
	  rclcpp::shutdown();
  }

  // printf("subscribe: [%s]\n", receiver.data.c_str());
  // printf("subscribe \n");

  if (initializer == 0) {
    // Initialize
 	  char init_num_char = *( msg->data.c_str());
 	  char *init_num_pt = &init_num_char;
 	  initializer = atoi(init_num_pt);
	  //	printf("initializer : %d \n", initializer);
 	  if (initializer == 1) {
 	    printf("start evaluation as a server \n");
  	}
  }

  if (initializer == 1) {
 	 // write (socket, "character", number of characters)
	 //	printf("write \n");
    write(sock, "x", 1);
  }
}

int set_bind_listen_accept_socket() {

  // Create socket
  printf("set \n");
  sock0 = socket(AF_INET, SOCK_STREAM, 0);
  if (sock0 < 0) {
  	perror("socket");
  	return 1;
  }

  // Socket settings
  addr.sin_family = AF_INET;
  addr.sin_port = htons(12345);
  addr.sin_addr.s_addr = INADDR_ANY;

  // Connect even if port is in TIME_WAIT state
  const int one = 1;
  setsockopt(sock0, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));

  printf("bind \n");
  if (bind(sock0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
  	perror("bind");
  	return 1;
  }

  // Prepare to wait for a connection request from a TCP client
  printf("listen \n");
  if (listen(sock0, 5) < 0) {
  	perror("listen");
  	return 1;
  }

  client_size = sizeof(client);
  printf("accept \n");
  printf("waiting for talker_client... \n");

  // Wait until there is communication from the client
  sock = accept(sock0, (struct sockaddr *)&client, &client_size);
  if (sock < 0) {
	 perror("accept");
	 return 1;
  }

  printf("accepted connection from %s, port=%d\n",
	inet_ntoa(client.sin_addr), ntohs(client.sin_port));

  return 0;
}


// int main(int argc, char **argv)
int main(int argc, char * argv[])
{
  mlockall(MCL_FUTURE);
  
  usleep(1000);					// avoid race condition
  sched_param  pri = {94}; 
  if (sched_setscheduler(0, SCHED_FIFO, &pri) == -1) {
  	perror("sched_setattr");
  	exit(EXIT_FAILURE);
  }

  rclcpp::init(argc, argv);

  auto node = rclcpp::Node::make_shared("listener");

  // QoS settings
  rmw_qos_profile_t custom_qos_profile;
  if (IS_RELIABLE_QOS == 1) {
	 custom_qos_profile = rmw_qos_profile_reliable;
  } else {
	 custom_qos_profile = rmw_qos_profile_best_effort;
  }
  
  auto sub = node->create_subscription<std_msgs::msg::String>("chatter", chatterCallback,  custom_qos_profile);

  #pragma GCC diagnostic pop

  // wait for establishing socket
  if (set_bind_listen_accept_socket() == 1) {
  	perror("set_bind_listen_accept_socket");
  	return 1;
  }
 
  //   ros::spin();
  rclcpp::spin(node);

  return 0;
}
