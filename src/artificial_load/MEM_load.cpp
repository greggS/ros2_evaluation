#include <chrono>
#include <memory>
#include<iostream>
#include <thread>

#include <sys/mman.h>		
#include <sched.h>


#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  auto node = rclcpp::Node::make_shared("MEM_load");
  rclcpp::WallRate loop_rate(5);

  unsigned long start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  int64_t * p;

  while((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - start) < 60000){ 

    if(p = (std::int64_t*)std::malloc(16000 * sizeof(std::int64_t))){

      for(int i = 0; i < 16000; i++) // populate the array
        p[i] = i;
    }

    std::free(p);

    // int *a = new int;   // allocating 
    // delete a;           // deallocating 
  }

  // while(rclcpp::ok()){

  //   rclcpp::spin_some(node);
  //   loop_rate.sleep();
  // }

  
  rclcpp::shutdown();

  return 0;
}