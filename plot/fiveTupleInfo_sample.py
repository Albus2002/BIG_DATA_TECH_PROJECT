import math
import random
file_path = "../output/fiveTupleInfo.out"
outfile_path = "../output/fiveTupleInfo_sample_100.out"
sample_prop = 0.01

with open(file_path, "r") as file:
    with open(outfile_path, "w") as ofile:
        for line in file:
            r = random.random()
            if r < sample_prop:
                ofile.write(line)

