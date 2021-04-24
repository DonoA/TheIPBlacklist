TheIPBlacklist
==
A project dedicated to making the smallest or fastest Blacklist possible.

Tested implementations:
 - EBVBL
 - bitset
 - cuckoo++
 - linear probing
 - robin hood hashset
 - cuckoo prefix

Cuckoo prefix is a new implementation designed for this paper.

# Conclusion
In this paper we reported on our implementation of Cuckoo Prefix, a highly space efficient data structure for the storage and checking of blocklists. We examined the throughput of several popular IP blocklisting approaches, and concluded that cuckoo prefix gave the best trade off between throughput and memory. In addition, we found that most modern blocklists are approaching a state in which a bit set is the optimal implementation for IP blocking. We also discovered that EBVBL provides extremely high throughput. For memory rich time sensitive applications, EBVBL provides a much smaller but equally as fast alternative to bit sets.

Interestingly, we found that packing data into bit segments, as cuckoo prefix does with its subnet information, makes it extremely costly to unpack and use. Further consideration is required as to how this data could be better stored and processed.

In addition, we discovered that linear probing and robin hood hashing were far less effective hash set implementations than expected. Despite their promising simplicity and cache locality optimization, under heavy loads they failed to compete with implementations like cuckoo hashing and EBVBL. 

The results of this project are published here:
> D. Allen and N. Shaghaghi, "Cuckoo Prefix: A Hash Set for Compressed IP Blocklists," 2020 30th International Telecommunication Networks and Applications Conference (ITNAC), 2020, pp. 1-8, doi: 10.1109/ITNAC50341.2020.9315074.
