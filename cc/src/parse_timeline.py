#- * - coding : utf - 8 - * -
"""Module for processing Facebook post export into text."""

import io
import re
import sys
import xml.etree.ElementTree

def isprint(c):
  """Returns true if the character is not unprintable ascii."""
  if c == 10 or c == 13:
    return True
  if c < 0x20:
    return False
  if c == 0x7F:
    return False
  return True

def asciify(s):
  """Removes all the non ascii chars from a string."""
  s = s.encode('ascii', 'xmlcharrefreplace')
  s = ''.join([chr(c) for c in s if isprint(c)])
  return s

def parsexml(argv):
  """Takes in a filename and returns an ElementTree for it."""
  f = open(argv[1], encoding='utf-8')
  s = f.read()
  s = asciify(s)
  s = io.StringIO(s)
  e = xml.etree.ElementTree.parse(s).getroot()
  f.close()
  return e

def allnodes(node):
  """Yields all nodes under and including the given one."""
  yield node
  for child in node:
    for n in allnodes(child):
      yield n

def striplinks(text):
  """ Replaces URLs in text with spaces. """
  match = re.search('https?://[^ ]*', text)
  while match:
#url = text[match.start() : match.end()]
    text = text[:match.start()] + ' ' + text[match.end():]
    match = re.search('https?://[^ ]*', text)
  return text

def dumpposts(argv):
  """Prints out a text version of all the posts in the given html file."""
  good = True
  for node in allnodes(parsexml(argv)):
    if 'class' in node.attrib and node.attrib['class'] == 'meta':
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
      print(striplinks(node.text))
      print('')
    else:
#There was a strange div that means it's not a regular post.
      good = False

dumpposts(sys.argv)
