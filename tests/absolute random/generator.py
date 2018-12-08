import random as r
import os
import string

#folder generating
folders = ['.']
folder_amount = r.randint(2, 10)
for i in range(folder_amount):
	folder = ''.join(r.choice(string.ascii_uppercase + string.digits) for _ in range(5))
	os.mkdir(folder)
	folders.append(folder)
sizes = [10, 20, 30, 40, 50, 60]

# copies generating
amount = r.randint(10, 20)
for i in range(amount):
	kbsize = r.choice(sizes)
	copies_amount = r.randint(0, 20)
	os.system('dd if=/dev/urandom of={where}.tst bs=1KB count={size}'.format(where=i, size=kbsize))
	for j in range(copies_amount):
		folder = r.choice(folders)
		to = os.path.join(folder, '{}-copy-j.tst'.format(i))
		os.system('cp {}.tst {}'.format(i, to))
