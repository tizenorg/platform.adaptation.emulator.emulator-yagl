BEGIN {
}
#/            value "name" string: "comp-slp";\n            value "enabled" uchar: 1;/ { print "            value \"name\" string: \"comp-slp\";\n            value \"enabled\" uchar: 0;" }


# Next
/            value "name" string: "comp-slp";/ { NEXT = 1; next }
/            value "name" string: "move-slp";/ { NEXT = 2; next }

# Next
{
    if (NEXT > 0) {
	if (NEXT == 1) {
	    print "            value \"name\" string: \"comp-slp\";"
	} else {
	    print "            value \"name\" string: \"move-slp\";"
	}
	print "            value \"enabled\" uchar: 0;"
	NEXT = 0
    } else {
	print
    }
}
