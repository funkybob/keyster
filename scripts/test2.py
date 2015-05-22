
from client import Keyser

k = Keyser()

for n in xrange(100000):
    key = '%08d' % n
    val = '%20d' % n

    print 'Set: %r == %r' % (key, val)
    k[key] = val

for n in xrange(100000):
    key = '%08d' % n

    print '%s == %s' % (key, k[key])

for n in xrange(100000):
    key = '%08d' % n

    del k[key]
