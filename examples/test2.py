#!/usr/bin/python
import os, sys
from subprocess import *
expected = [
 ('3+4*2/(1-5)^2^3 END'                                                       ,'3 4 2 * 1 5 - / + 2 xor 3 xor ,'),
 ('3+4*2/(1-5)^2^3 * multdiv( 1*2, (0x12 * 4)/2, 22&8*4) END'                 ,'3 4 2 * 1 5 - / + 2 xor 3 22 8 4 * and 0x12 4 * 2 / 1 2 * i*/ * xor ,'),
 ('1+2/3*4<<5>>6&7|8^9 END'                                                   ,'1 2 3 / 4 * + 5 shlv 6 shrv 7 and 8 9 xor or ,'),
 ('((0x123 * r | 1 ^ 2) < 3) && integer(float(4) / 0.0 / 0. * .0) != 0 END'   ,'0x123 r * 1 2 xor or 3 < 4 itof 0.0 f/ 0. f/ .0 f* ftoi 0 <> and ,')];
for x in expected:
  proc = Popen(["./parser"], stdin=PIPE, stdout=PIPE)
  out = proc.communicate(x[0])[0].strip()
  if out != x[1]:
    print "FAIL: '%s' but expected '%s'"%(out,x[1])
  else:
    print "PASS"
  
