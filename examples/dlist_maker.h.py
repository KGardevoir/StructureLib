#!/usr/bin/python

import argparse

parser = argparse.ArgumentParser(description="Generate various arduino argv based macros")
parser.add_argument('--max_args', default=10, type=int, help='Maximium number of VARGS')

args = parser.parse_args()

def make_dlist(levels=args.max_args):
  name = "DLIST_MAKE"
  def make_args(num):
    return ', '.join(["_%d"%x for x in range(1,num)])
  def make_func(num):
    def gen_code(num):
      return "dlist_append(%s,(Object*)_%d, FALSE)"%(gen_code(num-1) if num > 1 else "%s0()"%(name), num)
    return "#define %s%d("%(name,num) + make_args(num+1) + ") " + gen_code(num)
  return "#define %s(...) VARARG(%s, __VA_ARGS__)\n"%(name,name) + \
         "#define %s0() NULL\n"%(name) + \
         '\n'.join([make_func(x) for x in range(1,levels)]) + "\n"


with open('dlist_maker.h', "w+") as fout:
  fout.write('#define VA_NARGS_IMPL(' + ', '.join(["_%d"%x for x in range(0, args.max_args)]) + ', N, ...) N\n')
  fout.write('#define VA_NARGS(...) VA_NARGS_IMPL(X,##__VA_ARGS__, ' + ', '.join(["%d"%x for x in range(args.max_args-1, -1, -1)]) + ')\n')
  fout.write('#define VARARG_IMPL2(base, count, ...) base##count(__VA_ARGS__)\n')
  fout.write('#define VARARG_IMPL(base, count, ...) VARARG_IMPL2(base, count, __VA_ARGS__)\n')
  fout.write('#define VARARG(base, ...) VARARG_IMPL(base, VA_NARGS(__VA_ARGS__), __VA_ARGS__)\n\n')
  
  fout.write(make_dlist())
