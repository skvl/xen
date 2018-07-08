import re
from debian_linux.debian import Version


class VersionXen(Version):
    _version_xen_rules = r"""
        ^
        (?P<major>\d+)\.(?P<minor>\d+)(?:\.(?P<patch>\d+))?
        (?:
         ~pre(?:\+\d+\.(?P<pre_commit>[0-9a-f]{10}))
         |
         ~rc(?P<rc>\d+)(?:\+\d+\.(?P<rc_commit>[0-9a-f]{10}))?
        )?
        -
        (?P<debian_revision>[^-]+)
        $
        """
    _version_xen_re = re.compile(_version_xen_rules, re.X)

    def __init__(self, version):
        super(VersionXen, self).__init__(version)
        match = self._version_xen_re.match(version)
        if match is None:
            raise ValueError("Invalid debian xen version")
        d = match.groupdict()
        self.major = d['major']
        self.minor = d['minor']
        self.patch = d['patch']
        self.pre_commit = d['pre_commit']
        self.rc_commit = d['rc_commit']
        self.rc = d['rc']
        self.debian_revision = d['debian_revision']

        # Now find out which treeish in the upstream source we need to build
        # the orig tar from.

        # 1. pre version in between stable releases with explicit commit
        # e.g. 4.10.2~pre+1.25e0657ed4-1
        if self.pre_commit is not None:
            self.treeish = self.pre_commit
        # 2. explicit commit while in rc
        # e.g. 4.11.0~rc6+1.35fcb982ea-1~exp1
        elif self.rc_commit is not None:
            self.treeish = self.rc_commit
        # 3. release candidate
        # e.g. 4.11.0~rc7-1~exp1
        elif self.rc is not None:
            self.treeish = '%s.%s.%s-rc%s' % (self.major, self.minor, self.patch, self.rc)
        # 4. regular release, like 4.10.2 -> tag RELEASE-4.10.2
        else:
            self.treeish = 'RELEASE-%s.%s.%s' % (self.major, self.minor, self.patch)
