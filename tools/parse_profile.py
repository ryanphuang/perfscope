f = open('gccprofile', 'r')
pat = "/home/ryan/experiment/gcc-build/bin/gcc"
for line in f:
    line = line.strip()
    idx = line.find(pat)
    if idx == -1:
        continue
    func = line[idx + len(pat):].strip()
    print func.strip()
    #print line[:idx].split(" ")[0], func.strip()
f.close()
