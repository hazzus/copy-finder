import os

for i in os.walk('.'):
	if i[2] == 'generator.py' and len(i[1]) == 0:
		os.system('python3 {}'.format(os.path.join(i[0], i[2])))
