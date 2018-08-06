TheIPBlacklist
==
A project dedicated to making the smallest or fastest Blacklist possible

## Test Results:

Run on an amazon ec2 t2.medium instance:
- 2 Intel Xeon cores @ 2.5 Ghz
- 4 GB ram
- Ubuntu 16.04.5 LTS
- openjdk version 1.8.0_171
- x86_64 system, 64 bit java server vm

All memory statistics were collected using VisualVM's retained memory calculator.

### Against 100k IPs:

```
Simple String Blacklist:
	Insert Time:			242.692391 ms
	Test Containing Time:		140.516446 ms
	Test Not Containing Time:	121.086269 ms

	Blacklist size:			14,356,222 B
```

```
Simple Integer Blacklist:
	Insert Time:			316.5527 ms
	Test Containing Time:		160.64379 ms
	Test Not Containing Time:	136.652277 ms

	Blacklist size:			8,497,288 B
```

```
Integer Tree Blacklist:
	Insert Time:			371.211938 ms
	Test Containing Time:		218.42699 ms
	Test Not Containing Time:	355.902153 ms

	Blacklist size:			57,211,064 B
```

```
Integer Linear Tree Blacklist:
	Insert Time:			85940.384927 ms
	Test Containing Time:		141.680717 ms
	Test Not Containing Time:	137.224315 ms

	
	Blacklist size:			854,095 B
```

```
Primitive int HashSet Blacklist:
	Insert Time:			294.475684 ms
	Test Containing Time:		144.617966 ms
	Test Not Containing Time:	133.297255 ms

	
	Blacklist size:			520,068 B
```

### Against 2.5M IPs:

```
Simple String Blacklist:
        Insert Time:                    6035.583607 ms
        Test Containing Time:           3359.798708 ms
        Test Not Containing Time:       2363.103371 ms

	Blacklist size:			340,052,029 B
```

```
Simple Integer Blacklist:
        Insert Time:                    5335.77721 ms
        Test Containing Time:           3224.037185 ms
        Test Not Containing Time:       2983.105212 ms

	Blacklist size:			193,554,568 B
```

```
Integer Tree Blacklist:
        Insert Time:                    16425.36852 ms
        Test Containing Time:           60150.277735 ms
        Test Not Containing Time:       60618.023381 ms

	Blacklist size:			1,037,812,977 B
```

```
Integer Linear Tree Blacklist:
        Insert Time:                    17.08 Hours
        Test Containing Time:           10123.662025 ms
        Test Not Containing Time:       8727.557654 ms

	
	Blacklist size:			14,418,00 B
```

```
Primitive int HashSet Blacklist:
        Insert Time:                    3251.946185 ms
        Test Containing Time:           2912.345392 ms
        Test Not Containing Time:       3058.220489 ms
	
	Blacklist size:			13,000,068 B
```

## Conclusion:

From these results I think it is clear to see that the primitive int hash set is be best tool for the job. By removing references from the equation and using simple primitive types, the memory footprint was greatly reduced. The hash set of primitive integers could hold 2.5M ips in the same space as the 100k string based blacklist. That's 25 times better!

Interestingly, the string hash set ran faster than the primitive hash set which I would assume was because of two reasons:
- First, the string hash set uses the internal hash set class which I assume is far better optimized than my simplistic primitive set.
- Second, in some sense strings provide more data points when hashing than 32 bit integers which allows for lower levels of collision. When analyzing the primitive hash set in practice, I found it suffered badly from secondary clustering, a problem that I was unable to fix. Instead, I moved to a method called robinhood hashing which kept the maximum displacement between the item's hash position and its real location as low as possible. This provided almost no increase in performance (somehow) however it did allow me to increase the load of the hash set, conserving space, without serious degradation to performance. 