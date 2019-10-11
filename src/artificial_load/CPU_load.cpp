#include <chrono>
#include <memory>
#include<iostream>
#include <thread>

#include <sys/mman.h>		
#include <sched.h>


#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

void thread_load_function()
{ 
    unsigned long start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    float number = 1.5;

    while((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - start) < 60000)
      number*=number;  
      
    return;
}

int main(int argc, char * argv[])
{

  // sched_param  pri = {93}; 
  // if (sched_setscheduler(0, SCHED_FIFO, &pri) == -1) { // set FIFO scheduler
  // 	perror("sched_setattr");
  // 	exit(EXIT_FAILURE);
  // }

  rclcpp::init(argc, argv);

  auto node = rclcpp::Node::make_shared("CPU_load");
  rclcpp::WallRate loop_rate(5);

  unsigned concurentThreadsSupported = std::thread::hardware_concurrency();

  std::vector<std::thread> threads;
    for(unsigned i = 0; i < concurentThreadsSupported; ++i) {
        threads.push_back(std::thread(thread_load_function));
    }
  std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

  // while(rclcpp::ok()){

  //   rclcpp::spin_some(node);
  //   loop_rate.sleep();
  // }

  rclcpp::shutdown();
  return 0;
}