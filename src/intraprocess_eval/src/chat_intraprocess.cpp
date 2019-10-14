#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

#include <sstream>
#include <fstream>
#include <iostream>
#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <string>

#include <time.h>			// clock
#include <unistd.h>			// clock

#include <sys/mman.h>			// mlock
#include <sched.h>			// sched

#define EVAL_NUM 120
#define PUBLISH_Hz_millisec 100
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

static const rmw_qos_profile_t rmw_qos_profile_history = {
  RMW_QOS_POLICY_HISTORY_KEEP_LAST,
  100,							// depth option for HISTORY
  RMW_QOS_POLICY_RELIABILITY_RELIABLE,
  RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL
};

struct timespec tp1, tp2;
int i;
int count_talker =-1, count_listener = -1, count_end = 0, init_num_int;
int eval_init = 1;
int eval_loop_count = 0;

double publish_time[EVAL_NUM];
double subscribe_time[EVAL_NUM];

std::string s, bytedata;
std::string output_filename = "./evaluation/subscribe_time/subscribe_time_256byte.txt";
std_msgs::msg::String receiver;

FILE *fp1, *fp2;


std::string read_datafile(std::string message_filename){
  
  std::ifstream ifs(message_filename.c_str());
  if(ifs.fail()) {
	std::cerr << "data_*byte.txt do not exist.\n";
	exit(0);
  }

  std::string bytedata;
  getline(ifs, bytedata);

  return bytedata;
}

int eval_ros2(std::string message_filename, std::string output_filename, rclcpp::Publisher<std_msgs::msg::String>::SharedPtr chatter_pub){
  if( -1 < count_talker ){

	//std_msgs::msg::String::UniquePtr msg_pub(new std_msgs::msg::String());
	auto msg_pub = std_msgs::msg::String();	
	std::stringstream ss;
  
	ss << count_talker;
	s = ss.str() + bytedata;
	msg_pub.data = s;
  
	if(clock_gettime(CLOCK_REALTIME,&tp1) < 0){
	  perror("clock_gettime begin");
	  return 0;
	}
	publish_time[count_talker] = (double)tp1.tv_sec + (double)tp1.tv_nsec/ (double)1000000000L;
	
	//	printf("Published message address: 0x%" PRIXPTR "\n", reinterpret_cast<std::uintptr_t>(msg_pub.get()));
	
	//	printf("%18.9lf\n",publish_time[count_talker]);
	// printf("publish_time[%2d]:\t%18.9lf\n", count_talker, publish_time[count_talker]);
	// printf("I say: [%s]\n", msg->data.c_str());
	// printf("Time Span:\t%ld.%09ld\n", tp1.tv_sec, tp1.tv_nsec);

	chatter_pub->publish(msg_pub);
  }
  else if(count_talker == -1){
	 
	bytedata = read_datafile(message_filename.c_str());
  
  }
  
  if( count_talker == EVAL_NUM - 1){

	if((fp1 = fopen(output_filename.c_str(), "w")) != NULL){
	  for(i=0; i<EVAL_NUM; i++){

		if(fprintf(fp1, "%18.9lf\n", publish_time[i]) < 0){
		  printf("error : can't output publish_time.txt \n");
		}
	  }
	  fclose(fp1);
	}else{
	  printf("error : can't output publish_time.txt \n");	
	}
	
	count_talker = -2;
	eval_init = 1;
  }
  
  count_talker++;
  
  return 0;

}

// Node that produces messages.
struct Producer : public rclcpp::Node
{
  Producer(const std::string & name, const std::string & output)
	: Node(name)
  {

	this->declare_parameter("QoS_Policy");
  	int QoS_Policy;
  	this->get_parameter_or("QoS_Policy", QoS_Policy, 1);

	// QoS settings
	rmw_qos_profile_t custom_qos_profile;
	if (QoS_Policy == 1){
	  custom_qos_profile = rmw_qos_profile_reliable;
	}else if (QoS_Policy == 2){
	  custom_qos_profile = rmw_qos_profile_best_effort;
	} else if (QoS_Policy == 3){
	  custom_qos_profile = rmw_qos_profile_history;
	}
    // Create a publisher on the output topic.
    pub_ = this->create_publisher<std_msgs::msg::String>(output, custom_qos_profile);

    std::weak_ptr<std::remove_pointer<decltype(pub_.get())>::type> captured_pub = pub_;

    // Create a timer which publishes on the output topic at 10Hz.
    auto callback = [captured_pub]() -> void {
	  auto chatter_pub = captured_pub.lock();
	  if (!chatter_pub) {
		return;
	  }

	  if (eval_loop_count == 0) {
		if (eval_init == 1){
		  usleep(3000000);		// wait 3 sec for establishing listener
		  printf("start evaluation 256byte \n");
		  eval_init = 0;
		}
		eval_ros2("./evaluation/byte_data/data_256byte.txt", "./evaluation/publish_time/publish_time_256byte.txt", chatter_pub);
	  }

	  if (eval_loop_count == 1) {
		if (eval_init == 1) {
		  usleep(1000000);		// wait 1 sec to avoid race condition
		  printf("start evaluation 512byte \n");
		  eval_init = 0;
		}
		eval_ros2("./evaluation/byte_data/data_512byte.txt", "./evaluation/publish_time/publish_time_512byte.txt", chatter_pub);
	  }

	  if (eval_loop_count == 2) {
		if (eval_init == 1) {
		  usleep(1000000);
		  printf("start evaluation 1Kbyte \n");
		  eval_init = 0;
		}
		eval_ros2("./evaluation/byte_data/data_1Kbyte.txt", "./evaluation/publish_time/publish_time_1Kbyte.txt", chatter_pub);
	  }

	  if (eval_loop_count == 3) {
		if (eval_init == 1) {
		  usleep(1000000);
		  printf("start evaluation 2Kbyte \n");
		  eval_init = 0;
		}
		eval_ros2("./evaluation/byte_data/data_2Kbyte.txt", "./evaluation/publish_time/publish_time_2Kbyte.txt", chatter_pub);
	  }

	  if (eval_loop_count == 4) {
		if (eval_init == 1) {
		  usleep(1000000);
		  printf("start evaluation 4Kbyte \n");
		  eval_init = 0;
		}
		eval_ros2("./evaluation/byte_data/data_4Kbyte.txt", "./evaluation/publish_time/publish_time_4Kbyte.txt", chatter_pub);
	  }

	  if (eval_loop_count == 5) {
		if (eval_init == 1) {
		  usleep(1000000);
		  printf("start evaluation 8Kbyte \n");
		  eval_init = 0;
		}
		eval_ros2("./evaluation/byte_data/data_8Kbyte.txt", "./evaluation/publish_time/publish_time_8Kbyte.txt", chatter_pub);
	  }

	  if (eval_loop_count == 6) {
		if (eval_init == 1) {
		  usleep(1000000);
		  printf("start evaluation 16Kbyte \n");
		  eval_init = 0;
		}
		eval_ros2("./evaluation/byte_data/data_16Kbyte.txt", "./evaluation/publish_time/publish_time_16Kbyte.txt", chatter_pub);
	  }


	  if (eval_loop_count == 7) {
		if (eval_init == 1) {
		  usleep(1000000);
		  printf("start evaluation 32Kbyte \n");
		  eval_init = 0;
		}
		eval_ros2("./evaluation/byte_data/data_32Kbyte.txt", "./evaluation/publish_time/publish_time_32Kbyte.txt", chatter_pub);
	  }

	  if (eval_loop_count == 8) {
		if (eval_init == 1) {
		  usleep(1000000);
		  printf("start evaluation 64Kbyte \n");
		  eval_init = 0;
		}
	  	eval_ros2("./evaluation/byte_data/data_64Kbyte.txt", "./evaluation/publish_time/publish_time_64Kbyte.txt", chatter_pub);
	  }

	  if (eval_loop_count == 9) {
		if (eval_init == 1) {
		  usleep(1000000);
		  printf("start evaluation 128Kbyte \n");
		  eval_init = 0;
		}
		eval_ros2("./evaluation/byte_data/data_128Kbyte.txt", "./evaluation/publish_time/publish_time_128Kbyte.txt", chatter_pub);
	  }

	  if (eval_loop_count == 10) {
		if (eval_init == 1) {
		  usleep(1000000);
		  printf("start evaluation 256Kbyte \n");
		  eval_init = 0;
		}
		eval_ros2("./evaluation/byte_data/data_256Kbyte.txt", "./evaluation/publish_time/publish_time_256Kbyte.txt", chatter_pub);
	  }

	  	  if (eval_loop_count == 11) {
		if (eval_init == 1) {
		  usleep(1000000);
		  printf("start evaluation 512Kbyte \n");
		  eval_init = 0;
		}
  	eval_ros2("./evaluation/byte_data/data_512Kbyte.txt", "./evaluation/publish_time/publish_time_512Kbyte.txt", chatter_pub);
	  }

	  if (eval_loop_count == 12) {
		if (eval_init == 1) {
		  usleep(1000000);
		  printf("start evaluation 1Mbyte \n");
		  eval_init = 0;
		}
		eval_ros2("./evaluation/byte_data/data_1Mbyte.txt", "./evaluation/publish_time/publish_time_1Mbyte.txt", chatter_pub);
	  }

	  if (eval_loop_count == 13) {
		if (eval_init == 1) {
		  usleep(1000000);
		  printf("start evaluation 2Mbyte \n");
		  eval_init = 0;
		}
		eval_ros2("./evaluation/byte_data/data_2Mbyte.txt", "./evaluation/publish_time/publish_time_2Mbyte.txt", chatter_pub);
	  }

	  if (eval_loop_count == 14) {
		if (eval_init == 1) {
		  usleep(1000000);
		  printf("start evaluation 4Mbyte \n");
		  eval_init = 0;
		}
		eval_ros2("./evaluation/byte_data/data_4Mbyte.txt", "./evaluation/publish_time/publish_time_4Mbyte.txt", chatter_pub);
	  }

	  // followthrough transactions
	  if (eval_loop_count == 15){
		eval_loop_count++;
		printf("---end evaluation---\n");
		rclcpp::shutdown();
	  }

	};
	
	// timer_ = this->create_wall_timer(1_s	, callback);
    timer_ = this->create_wall_timer(std::chrono::milliseconds(PUBLISH_Hz_millisec)	, callback);
  }

  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

// Node that consumes messages.
struct Consumer : public rclcpp::Node
{
  Consumer(const std::string & name, const std::string & input)
	: Node(name)
  {

  	this->declare_parameter("QoS_Policy");
  	int QoS_Policy;
  	this->get_parameter_or("QoS_Policy", QoS_Policy, 1);

	// QoS settings
	rmw_qos_profile_t custom_qos_profile;
	if (QoS_Policy == 1){
	  custom_qos_profile = rmw_qos_profile_reliable;
	}else if (QoS_Policy == 2){
	  custom_qos_profile = rmw_qos_profile_best_effort;
	} else if (QoS_Policy == 3){
	  custom_qos_profile = rmw_qos_profile_history;
	}

    // Create a subscription on the input topic which prints on receipt of new messages.
    sub_ = this->create_subscription<std_msgs::msg::String>(input, [](std_msgs::msg::String::UniquePtr msg) {

		if( count_listener == -1 ){

		  // Initialize count_listener
		  char init_num_char = *( msg->data.c_str());
		  char *init_num_pt = &init_num_char;
		  count_listener = atoi(init_num_pt);
		  init_num_int = count_listener;    

		  // printf("first recieved number: %d \n\n", count_listener);
		  printf("message loss : %d \n", init_num_int);
		  printf("eval_loop %d \n", eval_loop_count);
   
		}
   usleep(100);
		// evaluation
		if( count_listener < EVAL_NUM-1 ){
	
		  receiver = *msg;
		  if(clock_gettime(CLOCK_REALTIME,&tp1) < 0){
			perror("clock_gettime begin");
		  }
		  subscribe_time[count_listener] = (double)tp1.tv_sec + (double)tp1.tv_nsec/ (double)1000000000L;

		  // printf(" Received message address: 0x%" PRIXPTR "\n", reinterpret_cast<std::uintptr_t>(msg.get()));

		  // printf("%18.9lf\n",subscribe_time[count_listener]);
		  // printf("subscribe_time[%2d]:\t%18.9lf\n", count_listener,subscribe_time[count_listener]);

		  // printf("I heard: [%s]\n", receiver.data.c_str());
		  // printf("I heard: [%c]\n",* ( msg->data.c_str()) );

		  // printf("Time Span:\t%ld.%09ld\n", tp1.tv_sec, tp1.tv_nsec);

		  count_listener++;

		}else if( count_listener == EVAL_NUM - 1 ){

		  receiver = *msg;
		  if(clock_gettime(CLOCK_REALTIME,&tp1) < 0){
			perror("clock_gettime begin");
		  }
		  subscribe_time[count_listener] = (double)tp1.tv_sec + (double)tp1.tv_nsec/ (double)1000000000L;

		  if((fp2 = fopen(output_filename.c_str(), "w")) != NULL){
	
			if(fprintf(fp2, "%d\n",init_num_int ) < 0){
			  printf("error : can't output subscribe_time_*byte.txt \n");
			}

			for(i=0; i<EVAL_NUM; i++){
			  if(fprintf(fp2, "%18.9lf\n", subscribe_time[i]) < 0){
				printf("error : can't output subscribe_time_*byte.txt \n");
				break;
			  }
			}

			printf("output data %d \n", eval_loop_count);

			fclose(fp2);
		  }else{
			printf("error : can't output subscribe_time_*byte.txt'");
		  }
	
		  count_listener = -1;
		  eval_loop_count++;
	
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
			count_listener = EVAL_NUM;
		  }
	
		}

	  }, 
	  custom_qos_profile);

  }

  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;
};

int main(int argc, char * argv[])
{
  mlockall(MCL_FUTURE);
  
  usleep(1000);
  sched_param  pri = {94}; 
  if (sched_setscheduler(0, SCHED_FIFO, &pri) == -1) {
  	perror("sched_setattr");
  	exit(EXIT_FAILURE);
  }

  setbuf(stdout, NULL);
  rclcpp::init(argc, argv);
  rclcpp::executors::SingleThreadedExecutor executor;

  auto producer = std::make_shared<Producer>("listener", "number");
  auto consumer = std::make_shared<Consumer>("talker", "number");

  #pragma GCC diagnostic pop

  executor.add_node(producer);
  executor.add_node(consumer);
  executor.spin();
  return 0;
}
