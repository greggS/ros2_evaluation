#!/bin/bash

declare -a package_names=("interprocess_eval")
declare -a talker_prefixes=("talker_interprocess")
declare -a listener_prefixes=("listener_interprocess")
declare -a rmw_suffixes=("fastrtps" "connext" "opensplice")

for ((i=0; i<${#package_names[@]}; i++))
do
	package_name="${package_names[$i]}"
	talker_prefix="${talker_prefixes[$i]}" 
	listener_prefix="${listener_prefixes[$i]}" 
	
	for ((j=0; j<${#rmw_suffixes[@]}; j++))
	do
		./clean_evaltime.bash

		rmw_suffix="${rmw_suffixes[$j]}"
		command_talker="ros2 run ${package_name} ${talker_prefix}__rmw_${rmw_suffix}_cpp"
		command_listener="ros2 run ${package_name} ${listener_prefix}__rmw_${rmw_suffix}_cpp"
		
		$command_listener &
		$command_talker &
		
		talker_pid=$!
		wait $talker_pid

		listener_pid=`ps | grep listener | grep -v grep | awk '{print $1}'`
		kill -9 $listener_pid

		./calculation_transport_time
		cp -r ./evaluation "./evaluation_${package_name}_${rmw_suffix}"
	done
done