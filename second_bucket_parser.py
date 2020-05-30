import re, sys

file = open("results.txt", "r")

results = file.read().split("==================================================")

file.close()

outfile = open("results_sb.csv", "w+")

ips_regex = r"IPs = (\d+)$"
impl_regex = r"Implementation = (.+)$"
second_bucket_queries_regex = r"Second Bucket Queries = ([\d\.]+)"

for result in results:
    ips_match = re.search(ips_regex, result, flags=re.MULTILINE)
    impl_match = re.search(impl_regex, result, flags=re.MULTILINE)
    sb_match = re.search(second_bucket_queries_regex, result, flags=re.MULTILINE)

    if sb_match is None:
        continue

    outfile.write(f"\"{impl_match.group(1)}\", {ips_match.group(1)}, {sb_match.group(1)}\n")

outfile.close()