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
        self.repo = repo

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

    def __call__(self):
        out = "../orig/%s" % self.orig_tar
        self.log("Generate tarball %s\n" % out)

        if self.options.tag:
            treeish = self.options.tag
        else:
            if self.changelog_entry.version.pre_commit:
                treeish = self.changelog_entry.version.pre_commit
            elif self.changelog_entry.version.rc_commit:
                treeish = self.changelog_entry.version.rc_commit
            else:
                treeish = 'RELEASE-%s' % self.version.upstream

        try:
            os.stat(out)
            raise RuntimeError("Destination already exists")
        except OSError: pass

        try:
            os.mkdir("../orig")
        except OSError:
            pass

        ga_exists = os.path.exists('.gitattributes')
        try:
            if ga_exists:
                os.rename('.gitattributes','.gitattributes.genorig-saved')
            with open('.gitattributes', 'w') as ga:
                print('* -export-subst\n', file=ga)
            with open(out, 'wb') as f:
                _cmd = ('git', 'archive', '--worktree-attributes', '--prefix', '%s/' % self.orig_dir, treeish)
                p1 = subprocess.Popen(_cmd, stdout=subprocess.PIPE, cwd=self.repo)
                subprocess.check_call(('xz', ), stdin=p1.stdout, stdout=f)
                if p1.wait():
                    raise RuntimeError
        except:
            os.unlink(out)
            raise

        try:
            if ga_exists:
                os.rename('.gitattributes.genorig-saved','.gitattributes')
            else:
                os.unlink('.gitattributes')
        except Exception: pass

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
    assert(len(args) == 1)
    Main(options, *args)()
