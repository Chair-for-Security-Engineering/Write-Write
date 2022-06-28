import subprocess
import re
from statistics import mean

ASSOC = 16

t = []
fails = 0
for i in range(500):
    try:
        out = subprocess.check_output(["sudo ./ev_sets"], shell=True).decode("utf-8")
        if "successfull" in out:
            victim_data = re.search(r'Cache Set:\s+(\d+), Cache Slice:\s+(\d+)', out)
            set = int(victim_data.group(1))
            slice = int(victim_data.group(2))
            success_checker = re.findall(f'Cache Set:\\s+({set}), Cache Slice:\\s+({slice})', out)
            if len(success_checker) != ASSOC + 1:
                fails += 1
            else:
                time = re.search('Evset took (\d+) seconds (\d+) milliseconds', out, re.IGNORECASE)
                if time:
                    t.append(int(time.group(1))*1000+int(time.group(2)))
        else:
            fails += 1
    except subprocess.CalledProcessError as e:
        fails += 1

print(f"Avverage: {mean(t)}, Fails: {fails}")
    
