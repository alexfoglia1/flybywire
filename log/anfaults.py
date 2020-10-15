lines = []
lines_speeds = []
with open("failures.log", "r") as f:
    lines = f.readlines()
with open("../bin/G18.txt", "r") as f:
    line = f.readline()
    while line:
        if line.startswith("$GPGLL"):
            lines_speeds.append(line)
        line = f.readline()

print("total gpgll lines: {}".format(len(lines_speeds)))

cntusr, cntcont, cntstop, cntint = 0, 0, 0, 0
for line in lines:
    if "SIGUSR1" in line:
        cntusr += 1
    elif "SIGCONT" in line:
        cntcont += 1
    elif "SIGSTOP" in line:
        cntstop += 1
    elif "SIGINT" in line:
        cntint += 1

ratiousr = cntusr / len(lines_speeds)
ratiocont = cntcont / len(lines_speeds)
ratiostop = cntstop / len(lines_speeds)
ratioint = cntint / len(lines_speeds)

print("SIGUSR1 ratio: {}".format(ratiousr))
print("SIGCONT ratio: {}".format(ratiocont))
print("SIGSTOP ratio: {}".format(ratiostop))
print("SIGINT ratio: {}".format(ratioint))
