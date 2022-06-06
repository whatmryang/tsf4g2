import os, sys


class mk_errno:

    def __init__(self, file):
        self.file = file
        self.read_file(file)

    def clear(self):
        self.err_dict = {}
        self.err_list = []
        self.file_line = []
        self.mod_name = ''
        self.var_head = ''

    def read_file(self, file):
        self.clear()

        mod_name = ''
        var_head = ''
        stat = ''

        fp = open(file, 'r')
        line = fp.readline()
        while line:
            if line.startswith('//-->error_table '):
                h, n, ios, boe = line.split()
                if not mod_name:
                    mod_name = n.lower()
                    var_head = (n + '_err_').upper()
                assert mod_name == n
                if not stat:
                    assert boe == 'begin'
                    assert ios in ('code', 'string')
                    stat = ios
                    self.file_line.append('//-->error_table %s' % stat)
                else:
                    assert boe == 'end'
                    assert ios == stat
                    stat = ''
            elif stat == 'code':
                k, v = line.strip().split(',')
                assert k.startswith(var_head)
                k, v = k.rstrip(), v.lstrip()
                assert v[:2] == '//'
                v = v[2:].lstrip()
                assert not self.err_dict.has_key(k)
                self.err_list.append(k)
                self.err_dict[k] = v
            elif not stat:
                self.file_line.append(line)
            line = fp.readline()
        fp.close()

        assert mod_name and not stat
        self.mod_name = mod_name
        self.var_head = var_head

    def add_line(self, line):
        if not (self.var_head and line.startswith(self.var_head)):
            return -1
        if self.err_dict.has_key(line):
            return 1
        sys.stderr.write('add entry %s: ' % line)
        info = sys.stdin.readline().strip()
        if not info:
            return 2
        self.err_dict[line] = info
        self.err_list.append(line)
        return 0

    def dump(self):
        r = []
        flag_head = '//-->error_table %s' % self.mod_name.lower()
        for i in self.file_line:
            if i.startswith('//-->error_table '):
                ios = i.split()[1]
                r.append('%s %s begin\n' % (flag_head, ios))
                if ios == 'code':
                    for c in self.err_list:
                        r.append('    %s,  // %s\n' % (c, self.err_dict[c]))
                else:
                    for c in self.err_list:
                        r.append('    "%s",\n' % self.err_dict[c])
                r.append('%s %s end\n' % (flag_head, ios))
            else:
                r.append(i)
        return ''.join(r)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.stderr.write("Usage: %s <head_file>\n" % sys.argv[0])
        sys.exit(1)

    obj = mk_errno(sys.argv[1])

    line = sys.stdin.readline()
    while line:
        sys.stderr.write('%d\n' % obj.add_line(line.strip()))
        line = sys.stdin.readline()

    print obj.dump()
