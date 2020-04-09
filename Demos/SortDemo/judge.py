import os;

for i in range(0, 5):
	os.system("./disk_gen 100000 > input_data")
	os.system("./a.out input_data 8 1024 output")
	os.system("./ans_build < input_data > answer")
	if (os.system("diff -Bb output answer")) :
		break
	print("pass")