#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <iostream>		// file io
#include <unistd.h>		// clock
#include <time.h>		// clock
#include <sys/mman.h>		// mlock
#include <sched.h>		// sched
#include <string>

#define EVAL_NUM 120
#define QoS_Policy 3 // 1 means "reliable", 0 means "best effort", 3 means "history"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

static const rmw_qos_profile_t rmw_qos_profile_reliable = {
  RMW_QOS_POLICY_HISTORY_KEEP_ALL,
  100,
  RMW_QOS_POLICY_RELIABILITY_RELIABLE,
  RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL
};

static const rmw_qos_profile_t rmw_qos_profile_best_effort = {
  RMW_QOS_POLICY_HISTORY_KEEP_LAST,
  1,
  RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT,
  RMW_QOS_POLICY_DURABILITY_VOLATILE
};

static const rmw_qos_profile_t rmw_qos_profile_history = {
  RMW_QOS_POLICY_HISTORY_KEEP_LAST,
  100,							// depth option for HISTORY
  RMW_QOS_POLICY_RELIABILITY_RELIABLE,
  RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL
};

int i, count = -1, init_num_int;
double  subscribe_time[EVAL_NUM];

struct timespec tp1;		// for clock

FILE *fp;			// for file io

// eval_loop_count is updated when evaluation ends in each data size.
// 0 means evaluation of 256 bytes, 1 means evaluation of 512 bytes, ... 
int eval_loop_count = 0;	

std::string output_filename = "./evaluation/subscribe_time/subscribe_time_256byte.txt";

void chatterCallback(const std_msgs::msg::String::SharedPtr msg){

  std::string termination = msg->data.c_str();
  if(termination.find("end") != std::string::npos){
	  printf("---end evaluation---\n");
	  rclcpp::shutdown();
  }
  
  if( count == -1 ){

	// Initialize count
	char init_num_char = *( msg->data.c_str());
	char *init_num_pt = &init_num_char;
	count = atoi(init_num_pt);
	init_num_int = count;	// if init_num_int is not 0, some messages are lost.

	// printf("first recieved number: %d \n\n", count);
	printf("message loss : %d \n", init_num_int);
	printf("eval_loop %d \n", eval_loop_count);
   
  }
   
  // evaluation
  if( count < EVAL_NUM-1 ){
	
	if(clock_gettime(CLOCK_REALTIME,&tp1) < 0){
	  perror("clock_gettime begin");
	}
	subscribe_time[count] = (double)tp1.tv_sec + (double)tp1.tv_nsec/ (double)1000000000L;

	// printf("%18.9lf\n",subscribe_time[count]);
	// printf("I heard: [%c]\n",* ( msg->data.c_str()) );
	// printf("I heard: [%s]\n", msg->data.c_str());
	// printf("Time Span:\t%ld.%09ld\n", tp1.tv_sec, tp1.tv_nsec);
	// printf("subscribe_time[%2d]:\t%18.9lf\n", count,subscribe_time[count]);

	// char* p = (char *) msg->data.c_str();
	// p++;
	// printf("I heard: [%c%c]\n",* ( msg->data.c_str()), *p);

	count++;

  }else if( count == EVAL_NUM - 1 ){ 

	if(clock_gettime(CLOCK_REALTIME,&tp1) < 0){
	  perror("clock_gettime begin");
	}
	subscribe_time[count] = (double)tp1.tv_sec + (double)tp1.tv_nsec/ (double)1000000000L;

	// Output subscribe_time[] collectively to subscribe_tim_*bytee.txt after evaluation 
	if((fp = fopen(output_filename.c_str(), "w")) != NULL){
	
	  // write init_num
	  if(fprintf(fp, "%d\n",init_num_int ) < 0){
		// Write error
		printf("error : can't output subscribe_time_*byte.txt'");
	  }

	  // write subscribe_time[]
	  for(i=0; i<EVAL_NUM; i++){
		if(fprintf(fp, "%18.9lf\n", subscribe_time[i]) < 0){
		  // Write error
		  printf("error : can't output subscribe_time_*byte.txt'");
		  break;
		}
	  }

	  // printf("output data %d \n", eval_loop_count);

	  fclose(fp);

	}else{
	  printf("error : can't output subscribe_time_*byte.txt'");
	}
	
	// Initialization of evaluation
	count = -1;					// initilize for next date size
	eval_loop_count++;				// update for next data size
	
	if( eval_loop_count == 1){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_512byte.txt";
	}else if( eval_loop_count == 2){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_1Kbyte.txt";
	}else if( eval_loop_count == 3){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_2Kbyte.txt";
	}else if( eval_loop_count == 4){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_4Kbyte.txt";
	}else if( eval_loop_count == 5){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_8Kbyte.txt";
	}else if( eval_loop_count == 6){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_16Kbyte.txt";
	}else if( eval_loop_count == 7){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_32Kbyte.txt";
	}else if( eval_loop_count == 8){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_64Kbyte.txt";
	}else if( eval_loop_count == 9){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_128Kbyte.txt";
	}else if( eval_loop_count == 10){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_256Kbyte.txt";
	}else if( eval_loop_count == 11){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_512Kbyte.txt";
	}else if( eval_loop_count == 12){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_1Mbyte.txt";
	}else if( eval_loop_count == 13){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_2Mbyte.txt";
	}else if( eval_loop_count == 14){
	  output_filename = "./evaluation/subscribe_time/subscribe_time_4Mbyte.txt";
	}else if( eval_loop_count == 15){
	  // End of measurement
	  count = EVAL_NUM;
	}
	
  }
}

int main(int argc, char * argv[])
{
  mlockall(MCL_FUTURE);		// lock all cached memory into RAM and prevent future dynamic memory allocations
  
  usleep(1000);
  sched_param  pri = {94}; 
  if (sched_setscheduler(0, SCHED_FIFO, &pri) == -1) { // set FIFO scheduler
  	perror("sched_setattr");
  	exit(EXIT_FAILURE);
  }

  rclcpp::init(argc, argv);

  auto node = rclcpp::Node::make_shared("listener");

  // QoS settings
  rmw_qos_profile_t custom_qos_profile;
  if( QoS_Policy == 1){
	custom_qos_profile = rmw_qos_profile_reliable;
  }
  else if( QoS_Policy == 2 ){
	custom_qos_profile = rmw_qos_profile_best_effort;
  }
  else if( QoS_Policy == 3){
	custom_qos_profile = rmw_qos_profile_history;
  }
  
  auto sub = node->create_subscription<std_msgs::msg::String>("chatter", chatterCallback, custom_qos_profile);
 
  printf("start evaluation\n");

  #pragma GCC diagnostic pop

  rclcpp::spin(node);

  return 0;
}
