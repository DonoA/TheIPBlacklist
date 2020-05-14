#!/bin/bash

result_file="results.txt"

for test in bitset_test cuckoo_prefix_test cuckoopp_test EBVBL_test linear_probe_test
do
  make $test
  for data_set in  ClientAttacks
  do
    echo "================ ./$test ../datasets/$data_set/ ===================" >> $result_file
    ./$test ../datasets/$data_set/ >> $result_file
  done
done