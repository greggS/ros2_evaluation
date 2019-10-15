#!/bin/bash

declare -a package_names=("interprocess_eval" "intraprocess_eval")
declare -a talker_prefixes=("talker_interprocess" "chat_intraprocess")
declare -a listener_prefixes=("listener_interprocess" "chat_intraprocess")
declare -a rmw_suffixes=("fastrtps" "opensplice" "connext")
declare -a qos_policies=("1" "2" "3")
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
		# Iterate over all QoS policies options (see nodes implementations)
		for ((k=0; k<${#qos_policies[@]}; k++))
		do
		
		#Create yaml parameter files
			cat <<EOF > params.yaml
talker:
 ros__parameters:
  QoS_Policy: ${qos_policies[$k]}
listener:
 ros__parameters:
  QoS_Policy: ${qos_policies[$k]}
EOF
			#Iterate over all artificial load scenarios
			for ((l=0; l<${#artificial_load_executables[@]}; l++))
			do
				./clean_evaltime.bash

				rmw_suffix="${rmw_suffixes[$j]}"
				command_talker="ros2 run ${package_name} ${talker_prefix}__rmw_${rmw_suffix}_cpp __params:=params.yaml"
				command_listener="ros2 run ${package_name} ${listener_prefix}__rmw_${rmw_suffix}_cpp __params:=params.yaml"
				
				artificial_load_executable="${artificial_load_executables[$l]}"
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

				# Kill artificial load executable only if it was previously started
				if [ "{$artificial_load_executable}" != "" ]; then
					artificial_load_pid=`ps | grep '{$artificial_load_executable}' | grep -v grep | awk '{print $1}'`
					kill -9 $artificial_load_pid
				fi

				./calculation_transport_time
				# Save results in a separate, uniquely named folder
				cp -r ./evaluation "./evaluation_${package_name}_${rmw_suffix}_${artificial_load_executable}_qos${qos_policies[$k]}"
			done
		done
	done
done
