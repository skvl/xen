#============================================================================
# This library is free software; you can redistribute it and/or
# modify it under the terms of version 2.1 of the GNU Lesser General Public
# License as published by the Free Software Foundation.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#============================================================================
# Copyright (C) 2005-2006 XenSource Inc.
#============================================================================


import os
import os.path
import sys
import xen.util.path


class _Path(object):
    def __init__(self, path=[]):
        self._path = path
    def __call__(self, name):
        for dir in self._path:
            real = os.path.join(dir, name)
            if os.path.exists(real):
                return real


path_bin = _Path([xen.util.path.PRIVATE_BINDIR, '/usr/lib/xen/bin', '/usr/sbin', '/sbin', '/usr/bin', '/bin'])
path_boot = _Path([xen.util.path.XENFIRMWAREDIR, '/usr/lib/xen/boot', '/boot'])

def execute(exe, args = None):
    exepath = path_bin(exe)
    a = [ exepath ]
    if args:
        a.extend(args)
    os.execv(exepath, a)

def xen_configdir():
    return xen.util.path.XEN_CONFIG_DIR

def scripts_dir():
    return xen.util.path.XEN_SCRIPT_DIR
