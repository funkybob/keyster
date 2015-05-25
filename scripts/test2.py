
from client import Keyser

k = Keyser()

print "SET values"
for n in xrange(100000):
    key = '%08d' % n
    val = '%20d' % n

    print '%r == %r\r' % (key, val),
    k[key] = val

print "\nGET values"
for n in xrange(100000):
    key = '%08d' % n

    val = k[key]
    assert val == '%20d' % n
    print '%r == %r\r' % (key, val),

print "\nDEL values"
for n in xrange(100000):
    key = '%08d' % n

    del k[key]
