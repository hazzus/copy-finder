import os

for i in range(30):
	os.system('dd if=/dev/urandom of={}-big.tst bs=1MB count={}'.format(i, i * 10))

