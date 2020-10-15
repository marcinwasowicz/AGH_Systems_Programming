import sys
from subprocess import call

for _ in range(int(sys.argv[2])):
    call([sys.argv[1]])

