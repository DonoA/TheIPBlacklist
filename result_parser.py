import re, sys

file = open("results.txt", "r")

results = file.read().split("==================================================")

file.close()

outfile = open("results.csv", "w+")

ips_regex = r"IPs = (\d+)$"
memory_regex = r"\[PASSED\] (\d+), (\d+), (\d+), (\d+)$"
time_regex = r"\[PASSED\] ([\d.]+), ([\d.]+)$"
impl_regex = r"Implementation = (.+)$"
load_regex = r"load factor = ([\d\.]+)$"

for result in results:
    ips_match = re.search(ips_regex, result, flags=re.MULTILINE)
    mem_match = re.search(memory_regex, result, flags=re.MULTILINE)
    time_match = re.search(time_regex, result, flags=re.MULTILINE)
    impl_match = re.search(impl_regex, result, flags=re.MULTILINE)
    load_match = re.search(load_regex, result, flags=re.MULTILINE)

    if mem_match is None:
        continue

    load = "N/A"

    if load_match is not None:
        load = load_match.group(1)

    outfile.write(f"\"{impl_match.group(1)}\", {ips_match.group(1)}, {mem_match.group(3)}, {time_match.group(2)}, {load}\n")

outfile.close()