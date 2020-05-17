#!/bin/bash

result_file="results.txt"

for test in cuckoo_prefix_test
do
  make $test
  for data_set in ServerAttacks MasterList ClientAttacks
  do
    echo "================ ./$test ../datasets/$data_set/ ===================" >> $result_file
    ./$test ../datasets/$data_set/ >> $result_file
  done
done