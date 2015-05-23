# Keyster

Experiments in key/value storage.

## Overview

Keyster is a trivial key/value store which talks over UDP.

## Protocol

Commands are sent to the server on socket 6347.

All commands currently follow the same basic format

    <cmd><key>[<value>]

All replies follow the same format.

The <cmd> is a single byte indicating the action:

- X: cause the server to exit.
- G: Get the value for the supplied key
- S: Set a key/value
- D: Delete a key.

Reply commands use the lower case of the request letter.

For commands requiring a Key, it is nul terminated.

For commands sending a Value, its size in inferred from the size of the UDP packet.  This practically limits the size of values to (64k - 1 - len(key) - 1).

### eXit

Tells the server to exit, discarding all data.

No key or value fields are needed for this.

### Get <key>

Returns the value for the given key, if any.  If no key exists, no data are returned.

    g<key>\x00

### Set <key> <value>

Sets the value for the given key, adding it if it doesn't already exist.

Returns the key and value

    s<key>\x00<value>

### Delete <key>

Deletes the key, returning the value if it existed.

    d<key>\x00
    d<key>\x00<value>

