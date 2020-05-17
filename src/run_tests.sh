#!/bin/bash

result_file="results.txt"

for test in EBVBL_test # bitset_test cuckoo_prefix_test cuckoopp_test robinhood_test linear_probe_test
do
  make $test
  for data_set in MasterList
  do
    echo "================ ./$test ../datasets/$data_set/ ===================" >> $result_file
    ./$test ../datasets/$data_set/ >> $result_file
  done
done