"""A ridiculous python script to make Makefiles."""

import os
import os.path
import platform

MAC_OS = platform.system() == 'Darwin'

#
# Build meta language
#

class SysLib:
  """A library installed on the system."""
  def __init__(self, includes=None, libs=None):
    self.includes = includes or []
    self.libs = libs or []
    self.default = False

class BrewLib(SysLib):
  """A library installed with brew."""
  def __init__(self, name):
    super().__init__(
        includes=['-I$(shell brew --prefix ' + name + ')/include'],
        libs=['-l' + name, '-L$(shell brew --prefix ' + name + ')/lib'])

class InstalledLib(SysLib):
  """A library installed in the normal lib path."""
  def __init__(self, name):
    super().__init__(
        includes=[],
        libs=['-l' + name])

# Go ahead and load protobuf, since we need it for proto rules.
if MAC_OS:
  PROTOBUF = BrewLib(name='protobuf')
  PROTOC = '$(shell brew --prefix protobuf)/bin/protoc'
  CLANG = 'clang-format'
else:
  PROTOBUF = InstalledLib(name='protobuf')
  PROTOC = 'protoc'
  CLANG = 'clang-format-3.8'

def all_deps(obj):
  """Recursively visits all deps of the given obj."""
  d = []
  if hasattr(obj, 'deps'):
    for d1 in obj.deps:
      d.append(d1)
      for d2 in all_deps(d1):
        d.append(d2)
  return d

class Library:
  """A library in this project consisting of a .h and .cc file."""
  def __init__(self, name, srcs=None, hdrs=None, deps=None):
    self.name = name
    self.srcs = srcs or []
    self.hdrs = hdrs or []
    self.deps = deps or []
    self.default = False
  def all_headers(self):
    """Returns all the hdrs attributes of this library and its deps."""
    hs = list(self.hdrs)
    for d in all_deps(self):
      if hasattr(d, 'hdrs'):
        for h in d.hdrs:
          hs.append(h)
    return hs
  def obj(self):
    """Returns the path of the object file for this library."""
    return 'obj/' + self.name + '.o'
  def all_includes(self):
    """Returns all includes for this library and its deps."""
    inc = []
    for d in all_deps(self):
      if hasattr(d, 'includes'):
        for i in d.includes:
          inc.append(i)
    return inc
  def make(self):
    """Returns the lines for the Makefile for this library."""
    inc = ' '.join(self.all_includes())
    if inc != '':
      inc = inc + ' '
    s = self.obj() + ': '
    s = s + ' '.join(self.srcs) + ' '
    s = s + ' '.join(self.all_headers()) + '\n'
    s = s + '\t' + 'mkdir -p obj && g++ -g -std=c++11 '
    s = s + inc
    s = s + '-Igen -o $@ -c ' + ' '.join(self.srcs) + '\n'
    return s

class ProtoSrc:
  """A target for generating .pb.cc and .pb.h files for a .proto."""
  def __init__(self, name):
    self.name = name
    self.default = False
  def make(self):
    """Returns the lines for the Makefile for this target."""
    s = ('gen/' + self.name + '.pb.cc gen/' + self.name + '.pb.h: src/' +
         self.name + '.proto\n')
    s = s + '\tmkdir -p gen && '
    s = s + PROTOC
    s = s + ' --proto_path=src --cpp_out=gen $^\n'
    return s

class ProtoLib(Library):
  """A target for creating a .o file from .pb.cc and .pb.h files."""
  def __init__(self, name, deps=None):
    super().__init__(
        name=name,
        srcs=['gen/' + name + '.pb.cc'],
        hdrs=['gen/' + name + '.pb.h'],
        deps=deps or [])

class Binary:
  """A target representing a binary."""
  def __init__(self, name, deps):
    self.name = name
    self.deps = deps
    self.default = True
  def all_obj(self):
    """Returns all objs from this target and its deps."""
    objs = []
    for d in all_deps(self):
      if hasattr(d, 'obj'):
        objs.append(d.obj())
    return objs
  def all_libs(self):
    """Returns all libs from this target and its deps."""
    libs = []
    for d in all_deps(self):
      if hasattr(d, 'libs'):
        for l in d.libs:
          libs.append(l)
    return libs
  def make(self):
    """Returns the lines for the Makefile for generating this target."""
    s = 'bin/' + self.name + ': '
    s = s + ' '.join(self.all_obj()) + '\n'
    s = s + '\tmkdir -p bin && g++ -g -o $@ $^ '
    s = s + ' '.join(self.all_libs())
    s = s + '\n'
    return s

#
# File processing
#

def includes_from_file(filename):
  """Returns all the files included from the given file."""
  f = open(filename, 'r')
  s = f.read()
  f.close()
  local = []
  system = []
  for line in s.split('\n'):
    if not line.startswith('#include '):
      continue
    line = line[9:].strip()
    if line[0] == '"':
      line = line[1:-1]
      local.append(line)
    else:
      line = line[1:-1]
      system.append(line)
  return local, system

def find_protos(targets):
  """Finds all the protos under src and adds their targets to targets."""
  for dirpath, _, filenames in os.walk('./src'):
    for filename in filenames:
      filename = os.path.join(dirpath, filename)
      if not filename.endswith('.proto'):
        continue
      name = filename[6:-6]
      src = ProtoSrc(name=name)
      targets[name + '_pb'] = src
      targets[name + '.o'] = ProtoLib(name, deps=[src, PROTOBUF])

def find_libs(targets, sysdeps):
  """Finds all the .h/.cc libs under src and adds their targets to targets."""
  added = []
  skipped = []
  for dirpath, _, filenames in os.walk('./src'):
    for filename in filenames:
      filename = os.path.join(dirpath, filename)
      if not filename.endswith('.h'):
        continue
      h_file = filename
      cc_file = filename[:-2] + '.cc'
      name = h_file[6:-2]
      if name + '.o' in targets:
        continue
      if not os.path.isfile(cc_file):
        raise Exception('.h files must have .cc files (for now)')
      h_local, h_sys = includes_from_file(h_file)
      cc_local, cc_sys = includes_from_file(cc_file)
      # TODO(klimt): cc file includes shouldn't really be transitive headers.
      local_includes = h_local + cc_local
      sys_includes = h_sys + cc_sys
      # Remove proto files.
      lib_includes = [x for x in local_includes if not x.endswith('.pb.h')]
      proto_includes = [x for x in local_includes if x.endswith('.pb.h')]
      # Remove this lib's header from itself.
      lib_includes = [x for x in lib_includes if x != (name + '.h')]
      deps = []
      # Look up the library includes, and skip if any aren't loaded yet.
      skip = False
      for include in lib_includes:
        if include[:-1] + 'o' in targets:
          dep = targets[include[:-1] + 'o']
          deps.append(dep)
        else:
          skipped.append(include)
          skip = True
      if skip:
        continue
      # proto deps
      for include in proto_includes:
        dep = targets[include[:-5] + '.o']
        deps.append(dep)
      # sys deps
      for include in sys_includes:
        for dep in sysdeps[include]:
          deps.append(dep)
      targets[name + '.o'] = Library(
          name=name,
          srcs=[cc_file],
          hdrs=[h_file],
          deps=deps)
      added.append(name)
  if skipped:
    if added:
      find_libs(targets, sysdeps)
    else:
      raise Exception('circular dependency detected with: %s' %
                      ' '.join(skipped))

def find_bins(targets, sysdeps):
  """Finds all the .cc bins under src and adds their targets to targets."""
  for dirpath, _, filenames in os.walk('./src'):
    for filename in filenames:
      filename = os.path.join(dirpath, filename)
      if not filename.endswith('.cc'):
        continue
      cc_file = filename
      h_file = filename[:-3] + '.h'
      name = h_file[6:-2]
      if os.path.isfile(h_file):
        continue
      local_includes, sys_includes = includes_from_file(cc_file)
      # Remove proto files.
      lib_includes = [x for x in local_includes if not x.endswith('.pb.h')]
      deps = []
      # lib deps
      for include in lib_includes:
        dep = targets[include[:-2] + '.o']
        deps.append(dep)
      # proto deps
      for include in local_includes:
        if include.endswith('.pb.h'):
          dep = targets[include[:-5] + '.o']
          deps.append(dep)
      # sys deps
      for include in sys_includes:
        for dep in sysdeps[include]:
          deps.append(dep)
      lib = Library(
          name=name,
          srcs=[cc_file],
          hdrs=[],
          deps=deps)
      targets[name + '.o'] = lib
      targets[name] = Binary(
          name=name,
          deps=[lib])

#
# System deps
#

if MAC_OS:
  GFLAGS = BrewLib(name='gflags')
  GLOG = BrewLib(name='glog')
  GTEST = BrewLib(name='gtest')
  GTEST_MAIN = InstalledLib('gtest_main')
  LEVELDB = BrewLib(name='leveldb')
  OPENSSL = SysLib(
      includes=['-I$(shell brew --prefix openssl)/include'],
      libs=['-lcrypto', '-L$(shell brew --prefix openssl)/lib'])
else:
  GFLAGS = InstalledLib('gflags')
  GLOG = InstalledLib('glog')
  GTEST = SysLib(includes=[], libs=['-lgtest', '-lpthread', '-lgtest_main'])
  GTEST_MAIN = SysLib()
  LEVELDB = InstalledLib('leveldb')
  OPENSSL = SysLib()

SYSDEPS = {
    'algorithm': [],
    'cmath': [],
    'cstring': [],
    'dirent.h': [],
    'errno.h': [],
    'fcntl.h': [],
    'fstream': [],
    'iostream': [],
    'map': [],
    'memory': [],
    'random': [],
    'stdio.h': [],
    'stdlib.h': [],
    'streambuf': [],
    'string': [],
    'sys/mman.h': [],
    'sys/stat.h': [],
    'sys/types.h': [],

    'gflags/gflags.h': [GFLAGS],
    'glog/logging.h': [GLOG],
    'google/protobuf/stubs/status.h': [PROTOBUF],
    'gtest/gtest.h': [GTEST, GTEST_MAIN],
    'leveldb/db.h': [LEVELDB],
    'leveldb/write_batch.h': [LEVELDB],
    'openssl/md5.h': [OPENSSL],
}

#
# Makefile generation
#
def main():
  """Prints a Makefile based on the files under src."""
  targets = {}
  find_protos(targets)
  find_libs(targets, SYSDEPS)
  find_bins(targets, SYSDEPS)

  # Print out the default rule with all binaries.
  bins = ['bin/' + targets[t].name for t in sorted(targets)
          if targets[t].default]
  bins = ' '.join(bins)
  print('all:', bins)

  print("""
  clean:
    rm -rf bin || true
    rm -rf obj || true
    rm -rf gen || true

  format:
    """ + CLANG + """ -i src/*

  .PRECIOUS: obj/%.o
  """)

  # Print out each target.
  for target in sorted(targets):
    print(targets[target].make())

main()
