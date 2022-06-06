#coding=gbk

import os
import basetemp

def conv(filename):
    data = basetemp.readFile(filename)
    text = ''
    for line in data.split('\n'):
        text += line + '<br>'
    text = '<tr><td><div class="article1">' + text + '</div>'
    basename = os.path.splitext(os.path.basename(filename))[0]
    target = os.path.join('htmls\\' + basename + '.htm')
    basetemp.replaceAndSave(text, [], target)

if __name__ == '__main__':
    conv(r'articles/contactus.txt')