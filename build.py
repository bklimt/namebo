# A ridiculous python script to make Makefiles

import os
import os.path

# TODO(klimt): Make it work on a mac.
macos = False

#
# Build meta language
#

class syslib:
  def __init__(self, includes=[], libs=[]):
    self.includes = includes
    self.libs = libs
    self.default = False

class brew(syslib):
  def __init__(self, name):
    super().__init__(
      includes = ['-I$(shell brew --prefix ' + name + ')/include'],
      libs = ['-l' + name, '-L$(shell brew --prefix ' + name + ')/lib'])

class pkg_config(syslib):
  def __init__(self, name):
    super().__init__(
      includes = ['$(pkg-config protobuf --cflags)'],
      libs = ['$(pkg-config protobuf --libs)'])

class installed(syslib):
  def __init__(self, name):
    super().__init__(
      includes = [],
      libs = ['-l' + name])

# Go ahead and load protobuf, since we need it for proto rules.
if macos:
  protobuf = brew(name='protobuf')
else:
  protobuf = installed(name='protobuf')

def all_deps(obj):
  d = []
  if hasattr(obj, 'deps'):
    for d1 in obj.deps:
      d.append(d1)
      for d2 in all_deps(d1):
        d.append(d2)
  return d

class library:
  def __init__(self, name, srcs=[], hdrs=[], deps=[]):
    self.name = name
    self.srcs = srcs
    self.hdrs = hdrs
    self.deps = deps
    self.default = False
  def all_headers(self):
    hs = list(self.hdrs)
    for d in all_deps(self):
      if hasattr(d, 'hdrs'):
        for h in d.hdrs:
          hs.append(h)
    return hs
  def obj(self):
    return 'obj/' + self.name + '.o'
  def all_includes(self):
    all = []
    for d in all_deps(self):
      if hasattr(d, 'includes'):
        for i in d.includes:
          all.append(i)
    return all
  def make(self):
    inc = ' '.join(self.all_includes())
    if inc != '':
      inc = inc + ' '
    s = self.obj() + ': ' 
    s = s + ' '.join(self.srcs) + ' '
    s = s + ' '.join(self.all_headers()) + '\n'
    s = s + '\t' + 'mkdir -p obj && g++ -std=c++11 '
    s = s + inc
    s = s + '-Igen -o $@ -c ' + ' '.join(self.srcs) + '\n'
    return s

class proto_src:
  def __init__(self, name):
    self.name = name
    self.default = False
  def make(self):
    s = 'gen/' + self.name + '.pb.cc gen/' + self.name + '.pb.h: src/' + self.name + '.proto\n'
    s = s + '\tmkdir -p gen && '
    #TODO(klimt): Put protoc here
    s = s + 'protoc'
    s = s + ' --proto_path=src --cpp_out=gen $^\n'
    return s

class proto_lib(library):
  def __init__(self, name, deps=[]):
    super().__init__(
      name = name,
      srcs = ['gen/' + name + '.pb.cc'],
      hdrs = ['gen/' + name + '.pb.h'],
      deps = deps)

class binary:
  def __init__(self, name, deps):
    self.name = name
    self.deps = deps
    self.default = True
  def all_obj(self):
    all = []
    for d in all_deps(self):
      if hasattr(d, 'obj'):
        all.append(d.obj())
    return all
  def all_libs(self):
    all = []
    for d in all_deps(self):
      if hasattr(d, 'libs'):
        for l in d.libs:
          all.append(l)
    return all
  def make(self):
    s = 'bin/' + self.name + ': '
    s = s + ' '.join(self.all_obj()) + '\n'
    s = s + '\tmkdir -p bin && g++ -o $@ $^ '
    s = s + ' '.join(self.all_libs())
    s = s + '\n'
    return s

#
# File processing
#

def includes_from_file(filename):
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
  for dirpath, dirnames, filenames in os.walk('./src'):
    for filename in filenames:
      filename = os.path.join(dirpath, filename)
      if not filename.endswith('.proto'):
        continue
      name = filename[6:-6]
      src = proto_src(name=name)
      targets[name + '_pb'] = src
      targets[name + '.o'] = proto_lib(name, deps=[src, protobuf])


# returns True if it needs to be run again.
def find_libs(targets, sysdeps):
  added = []
  skipped = []
  libs = []
  for dirpath, dirnames, filenames in os.walk('./src'):
    for filename in filenames:
      filename = os.path.join(dirpath, filename)
      if not filename.endswith('.h'):
        continue
      h_file = filename
      cc_file = filename[:-2] + '.cc'
      name = h_file[6:-2]
      if (name + '.o') in targets:
        continue
      if not os.path.isfile(cc_file):
        raise Exception('.h files must have .cc files (for now)')
      h_local, h_sys = includes_from_file(h_file)
      cc_local, cc_sys = includes_from_file(cc_file)
      # TODO(klimt): This is kinda wrong, because cc file includes shouldn't be transitive headers.
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
        if (include[:-1] + 'o') in targets:
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
      targets[name + '.o'] = library(
        name = name,
        srcs = [cc_file],
        hdrs = [h_file],
        deps = deps)
      added.append(name)
  if skipped:
    if added:
      return find_libs(targets, sysdeps)
    else:
      raise Exception('circular dependency detected with: %s' % ' '.join(skipped))

def find_bins(targets, sysdeps):
  for dirpath, dirnames, filenames in os.walk('./src'):
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
      lib = library(
        name = name,
        srcs = [cc_file],
        hdrs = [],
        deps = deps)
      targets[name + '.o'] = lib
      targets[name] = binary(
        name = name,
        deps = [lib])

#
# System deps
#

# TODO(klimt): Enable mac stuff
if macos:
  gflags = brew(name='gflags')
  glog = brew(name='glog')
  leveldb = brew(name='leveldb')
  gtest = brew(name='gtest')
  openssl = syslib(
    includes=['-I$(shell brew --prefix openssl)/include'],
    libs=['-lcrypto', '-L$(shell brew --prefix openssl)/lib'])
else:
  gflags = installed('gflags')
  glog = installed('glog')
  gtest = syslib(includes=[], libs=['-lgtest', '-lpthread', '-lgtest_main'])
  leveldb = installed('leveldb')
  openssl = syslib()

sysdeps = {
  'cstring': [],
  'dirent.h': [],
  'errno.h': [],
  'fcntl.h': [],
  'fstream': [],
  'iostream': [],
  'map': [],
  'memory': [],
  'stdio.h': [],
  'stdlib.h': [],
  'streambuf': [],
  'string': [],
  'sys/mman.h': [],
  'sys/stat.h': [],
  'sys/types.h': [],

  'gflags/gflags.h': [gflags],
  'glog/logging.h': [glog],
  'google/protobuf/stubs/status.h': [protobuf],
  'gtest/gtest.h': [gtest],
  'leveldb/db.h': [leveldb],
  'leveldb/write_batch.h': [leveldb],
  'openssl/md5.h': [openssl],
}

#
# Makefile generation
#

# TODO(klimt): Put bins in there.

targets = {}
find_protos(targets)
find_libs(targets, sysdeps)
find_bins(targets, sysdeps)

# Print out the default rule with all binaries.
bins = ['bin/' + targets[t].name for t in targets if targets[t].default]
bins = ' '.join(bins)
print('all:', bins)

print("""
clean:
	rm -rf bin || true
	rm -rf obj || true
	rm -rf gen || true

format:
	clang-format-3.8 -i src/*

.PRECIOUS: obj/%.o
""")

# Print out each target.
for target in targets:
  print(targets[target].make())