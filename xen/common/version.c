#include <xen/compile.h>
#include <xen/version.h>

const char *xen_compile_date(void)
{
    return XEN_COMPILE_DATE;
}

const char *xen_compile_time(void)
{
    return XEN_COMPILE_TIME;
}

const char *xen_compile_system_distribution(void)
{
    return XEN_COMPILE_SYSTEM_DISTRIBUTION;
}

const char *xen_compile_system_maintainer_local(void)
{
    return XEN_COMPILE_SYSTEM_MAINTAINER_LOCAL;
}

const char *xen_compile_system_maintainer_domain(void)
{
    return XEN_COMPILE_SYSTEM_MAINTAINER_DOMAIN;
}

const char *xen_compile_system_version(void)
{
    return XEN_COMPILE_SYSTEM_VERSION;
}

const char *xen_compiler(void)
{
    return XEN_COMPILER;
}

unsigned int xen_major_version(void)
{
    return XEN_VERSION;
}

unsigned int xen_minor_version(void)
{
    return XEN_SUBVERSION;
}

const char *xen_extra_version(void)
{
    return XEN_EXTRAVERSION;
}

const char *xen_changeset(void)
{
    return XEN_CHANGESET;
}

