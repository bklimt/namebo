#/usr/bin/python3

import datetime
import io
import sys
import xml.etree.ElementTree

def divs(s):
  while len(s):
    div = s.find('<div')
    if div < 0:
      return
    s = s[div:]
    if s[:12] == '<div class="':
      openq = s.find('"') + 1
      s = s[openq:]
      closeq = s.find('"')
      if closeq < 0:
        print('Missing close quote in class.')
        sys.exit(-1)
      c = s[:closeq]
      s = s[closeq + 1:]
    elif s[:5] == '<div>':
      c = ''
      s = s[4:]
    else:
      print('Malformed div')
      print(s[:80])
      sys.exit(-1)
    if s[0] != '>':
      print('Expected > after class.')
      sys.exit(- 1)
    s = s[1:]
    end = s.find('</div>')
    if end < 0:
      print('Missing close div tag.')
      print(s[:100])
      sys.exit(-1)
    t = s[:end]
    s = s[end:]
    yield(c, t)

def isdate(s) :
  # Saturday, December 9, 2017 at 7 : 43pm PST
  fmt = '%A, %B %d, %Y at %I:%M%p %Z'
  try:
    datetime.datetime.strptime(s, fmt)
    return True
  except ValueError:
    return False

def isprint(c):
  if c < 0x20:
    return False
  if c == 0x7F:
    return False
  return True

def ascii(s):
  s = s.encode('ascii', 'xmlcharrefreplace')
  s = ''.join([chr(c) for c in s if isprint(c)])
  return s

def posts(s):
  for c, t in divs(s):
    if c != 'meta' and c != 'comment' and c != '':
      continue
    if c == '':
      yield t
    elif c == 'meta':
      pass
    elif c == 'comment':
      pass

def printposts(argv):
  f = open(argv[1])
  html = f.read()
  f.close()
  for t in posts(html):
    print(t[:80])

def fixnewlines(s):
  return '&#10;'.join(s.split('\n'))

def parsexml(argv):
  f = open(argv[1], encoding='utf-8')
  s = fixnewlines(f.read())
  s = io.StringIO(ascii(s))
  e = xml.etree.ElementTree.parse(s).getroot()
  f.close()
  return e

def allnodes(node, parents=None):
  if not parents:
    parents = []
  yield node, parents
  parents.append(node)
  for child in node:
    for n, p in allnodes(child, parents):
      yield n, p
  parents.pop()

def dumpposts(argv):
  good = True
  for node, parents in allnodes(parsexml(argv)):
    if 'class' in node.attrib and node.attrib['class'] == 'meta':
      # print(node.text)
      # print(node.tag, node.attrib, parents)
      # print('')
      good = True
    elif 'class' in node.attrib and node.attrib['class'] == 'comment':
      if not good:
        continue
      if not node.text:
        continue
      if node.text.find('\ufffd') >= 0:
        continue
      if node.text[0:4].lower() == '[fb]':
        continue
      print(node.text)
      # print(node.tag, node.attrib, parents)
      print('')
     else:
      # print('strange div')
      # print('')
      good = False

dumpposts(sys.argv)