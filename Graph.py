import matplotlib.pyplot as plt
import os, fnmatch

Filenames=["transport_time_256byte.txt", "transport_time_512byte.txt", "transport_time_1Kbyte.txt", "transport_time_2Kbyte.txt", "transport_time_4Kbyte.txt", "transport_time_8Kbyte.txt", "transport_time_16Kbyte.txt", "transport_time_32Kbyte.txt", "transport_time_64Kbyte.txt", "transport_time_128Kbyte.txt", "transport_time_256Kbyte.txt", "transport_time_512Kbyte.txt", "transport_time_1Mbyte.txt", "transport_time_2Mbyte.txt", "transport_time_4Mbyte.txt"]

Filenames_small=["transport_time_256byte.txt", "transport_time_512byte.txt", "transport_time_1Kbyte.txt", "transport_time_2Kbyte.txt", "transport_time_4Kbyte.txt", "transport_time_8Kbyte.txt", "transport_time_16Kbyte.txt", "transport_time_32Kbyte.txt", "transport_time_64Kbyte.txt"]

Filenames_big=["transport_time_64Kbyte.txt", "transport_time_128Kbyte.txt", "transport_time_256Kbyte.txt", "transport_time_512Kbyte.txt", "transport_time_1Mbyte.txt", "transport_time_2Mbyte.txt", "transport_time_4Mbyte.txt"]

y1 = ["256", "512", "1K", "2K", "4K", "8K", "16K", "32K", "64K"]
y2 = ["64K", "128K", "256K", "512K", "1M", "2M", "4M"]

Means_small = []

for filename in Filenames_small:
	path = "./evaluation/transport_time/" + filename
	file_in = open(path,'r')
	x = []
	mean=0
	for y in file_in.read().split('\n'):
		if y:
			x.append(float(y))
	for element in x:
		mean = mean + element
	mean = mean/len(x) * 1000
	Means_small.append(mean)

plt.figure(0)
plt.ylabel('Latency [ms]')
plt.xlabel('Data Size [byte]')
plt.plot(y1, Means_small, color='blue', marker='o', linestyle='dashed', linewidth=0.8, markersize=3)
#plt.show()

Means_big = []

for filename in Filenames_big:
	path = "./evaluation/transport_time/" + filename
	file_in = open(path,'r')
	x = []
	mean=0
	for y in file_in.read().split('\n'):
		if y:
			x.append(float(y))
	for element in x:
		mean = mean + element
	mean = mean/len(x) * 1000
	Means_big.append(mean)

plt.figure(1)
plt.ylabel('Latency [ms]')
plt.xlabel('Data Size [byte]')
plt.plot(y2, Means_big, color='red', marker='o', linestyle='dashed', linewidth=0.8, markersize=3)
plt.show()
