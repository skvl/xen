#!/usr/bin/env python3

import sys
sys.path.append(sys.path[0] + '/../lib/python')

import itertools
import os, os.path
import shutil
import subprocess

from debian_xen.debian import VersionXen
from debian_linux.debian import Changelog


class Main(object):
    log = sys.stdout.write

    def __init__(self, options, repo):
        self.options = options

        self.changelog_entry = Changelog(version=VersionXen)[0]
        self.source = self.changelog_entry.source
        self.version = self.changelog_entry.version

        if options.override_version:
            self.version = VersionXen('%s-0' % options.override_version)

        if options.component:
            self.orig_dir = options.component
            self.orig_tar = '%s_%s.orig-%s.tar.xz' % (self.source, self.version.upstream, options.component)
        else:
            self.orig_dir = '%s-%s' % (self.source, self.version.upstream)
            self.orig_tar = '%s_%s.orig.tar.xz' % (self.source, self.version.upstream)
            if options.tag is None:
                options.tag = 'RELEASE-' + self.version.upstream

    def __call__(self):
        out = "../orig/%s" % self.orig_tar
        self.log("Generate tarball %s\n" % out)

        try:
            os.stat(out)
            raise RuntimeError("Destination already exists")
        except OSError: pass

        try:
            with open(out, 'wb') as f:
                tag = self.options.tag or 'HEAD'
                p1 = subprocess.Popen(('git', 'archive', '--prefix', '%s/' % self.orig_dir, tag), stdout=subprocess.PIPE)
                subprocess.check_call(('xz', ), stdin=p1.stdout, stdout=f)
                if p1.wait():
                    raise RuntimeError
        except:
            os.unlink(out)
            raise

        try:
            os.symlink(os.path.join('orig', self.orig_tar), os.path.join('..', self.orig_tar))
        except OSError:
            pass


if __name__ == '__main__':
    from optparse import OptionParser
    p = OptionParser(prog=sys.argv[0], usage='%prog [OPTION]... DIR')
    p.add_option('-c', '--component', dest='component')
    p.add_option('-t', '--tag', dest='tag')
    p.add_option('-V', '--override-version', dest='override_version')
    options, args = p.parse_args()
    if len(args) != 1:
        raise RuntimeError
    Main(options, *args)()
