import os


for j in range(100):
	os.mkdir('{}'.format(j))
for i in range(10):
	os.system('dd if=/dev/urandom of={}-small.tst bs=1KB count={}'.format(i, 20))
	for j in range(i * 10):
		os.system('cp {i}-small.tst {j}/{i}-small-copy-{j}.tst'.format(i=i, j=j))
