#!/bin/bash

declare -a package_names=("interprocess_eval" "intraprocess_eval")
declare -a talker_prefixes=("talker_interprocess" "chat_intraprocess")
declare -a listener_prefixes=("listener_interprocess" "chat_intraprocess")
declare -a rmw_suffixes=("fastrtps" "opensplice" "connext")
declare -r artificial_load_package="artificial_load"
declare -a artificial_load_executables=("") #\
#"CPU_load" \
#"Mem_load")

# Iterate over all packages
for ((i=0; i<${#package_names[@]}; i++))
do
	package_name="${package_names[$i]}"
	talker_prefix="${talker_prefixes[$i]}" 
	listener_prefix="${listener_prefixes[$i]}" 
	
	# Iterate over all DDS options
	for ((j=0; j<${#rmw_suffixes[@]}; j++))
	do
		#Iterate over all artificial load scenarios
		for ((k=0; k<${#artificial_load_executables[@]}; k++))
		do
			./clean_evaltime.bash

			rmw_suffix="${rmw_suffixes[$j]}"
			command_talker="ros2 run ${package_name} ${talker_prefix}__rmw_${rmw_suffix}_cpp"
			command_listener="ros2 run ${package_name} ${listener_prefix}__rmw_${rmw_suffix}_cpp"
			
			artificial_load_executable="${artificial_load_executables[$k]}"
			# Empty string means no load
			if [ "${artificial_load_executable}" != "" ]; then
				command_artificial_load="ros2 run ${artificial_load_package} ${artificial_load_executable}"
				$command_artificial_load &
			fi 
			
			# Execute both listener and talker only if necessary (intraprocess handling)
			if [ "${command_listener}" != "${command_talker}" ]; then
				$command_listener &
			fi
			$command_talker &
			
			talker_pid=$!
			wait $talker_pid

			listener_pid=`ps | grep listener | grep -v grep | awk '{print $1}'`
			kill -9 $listener_pid

			# Kill artificial load executable only if it was previously started
			if [ "{$artificial_load_executable}" != "" ]; then
				artificial_load_pid=`ps | grep '{$artificial_load_executable}' | grep -v grep | awk '{print $1}'`
				kill -9 $artificial_load_pid
			fi

			./calculation_transport_time
			# Save results in a separate, uniquely named folder
			cp -r ./evaluation "./evaluation_${package_name}_${rmw_suffix}_${artificial_load_executable}"
		done
	done
done
