import sys
from subprocess import call

for _ in range(10000):
    call([sys.argv[1]])

