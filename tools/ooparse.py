f = open('opresult', 'r')
pat = "/home/ryan/experiment/5.1-build/libexec/mysqld"
for line in f:
    line = line.strip()
    idx = line.find(pat)
    if idx == -1:
        continue
    func = line[idx + len(pat):]
    #parts = line.split(" ")
    #print line[:idx].split(" ")[0], func.strip()
    print func.strip()
f.close()
