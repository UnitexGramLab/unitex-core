# -*- coding: utf-8 -*-

#
# Pretty-printers for the Unitex C++ NLP Core (https://unitexgramlab.org)
# https://github.com/UnitexGramLab/unitex-core
# cristian.martinez@unitexgramlab.org (martinec)
#

import itertools
import sys
from .utils import *

@add_regex_printer
class unichar:
  """Pretty Printer for unichar internal string"""
  printer_name = 'unichar'
  min_supported_version = (4, 0, 0)
  max_supported_version = last_supported_unitex_version
  template_name = 'unitex::unichar'

  def __init__(self, val):
    self.val = val

  def to_string(self):
    #return "%s '%s'" % (self.val, char_make(self.val))
    c = "%s '%s'" % (int(self.val), char_make(self.val))
    return c

@add_printer
class Ustring:
  """Pretty Printer for Ustring internal string"""
  printer_name = 'Ustring'
  min_supported_version = (4, 0, 0)
  max_supported_version = last_supported_unitex_version
  template_name = 'unitex::Ustring'

  def __init__(self, val):
    self.val = val
    self.length = int(val['len'])
    self.capacity = int(val['size'])
    self.buffer = val['str']

  def to_string(self):
    try:
      bufferPtr = self.buffer.cast(gdb.lookup_type("unitex::unichar").pointer())
    except Exception:
      return '[uninitialized]'

    if self.buffer == 0x0:
      return '[nil]'

    if self.length == 0:
      return '[empty]'

    s = '%s"%s" [len:%d,capacity:%d]' % (
          string_address(self.buffer),
          string_make(self.buffer, self.length),
          self.length,
          self.capacity)

    return s

@add_printer
class UnitexString:
  """Pretty Printer for UnitexString internal string"""
  printer_name = 'UnitexString'
  min_supported_version = (4, 0, 0)
  max_supported_version = last_supported_unitex_version
  template_name = 'unitex::UnitexString'

  def __init__(self, val):
    self.val = val
    self.data = val['data_'].dereference()
    self.members = []
    self.members.append(('data_', self.data))

  def to_string(self):
    storage = self.val.cast(self.data.type)
    printer = gdb.default_visualizer(self.data)
    
    if printer is None:
      return storage

    s = printer.to_string()
    return s
