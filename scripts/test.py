
from client import Keyser


k = Keyser()

import random
import string

keychars = 'ABCDEFGHIJ'

for n in xrange(1000000):
    key = ''.join(random.sample(keychars, 4))
    value = ''.join(random.sample(string.printable, 64))

    action = random.randint(0, 3)
    if action == 0:
        k[key] = value
    elif action == 1:
        k[key]
    elif action == 2:
        del k[key]
