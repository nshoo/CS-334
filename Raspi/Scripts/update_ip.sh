#!/usr/bin/env bash

wget -q --spider http://google.com

# Wait for internet connection
while [ $? -ne 0 ]; do
	sleep 10
	wget -q --spider http://google.com
done

# Write ip info to file
ifconfig | grep inet | grep broadcast > /home/student334/CS-334/Raspi/ip.md

# Push ip file to github repo
git -C /home/student334/CS-334/ add Raspi/ip.md
git -C /home/student334/CS-334/ commit -m "Update raspi ip"
git -C /home/student334/CS-334/ push -f origin main
