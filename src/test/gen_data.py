import random

random.seed(31415)
file = 'data.txt'
count_lines = 100000

unique = set()
with open(file, 'w') as f:
    for i in range(count_lines):
        r = random.randrange(0, 1 << 24)
        line = f'{r}\n'
        f.write(line)
        unique.add(r)

print('generated:', file)
print('count lines:', count_lines)
print('count unique:', len(unique))
