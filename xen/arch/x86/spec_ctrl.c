/******************************************************************************
 * arch/x86/spec_ctrl.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (c) 2017-2018 Citrix Systems Ltd.
 */
#include <xen/errno.h>
#include <xen/init.h>
#include <xen/lib.h>

#include <asm/microcode.h>
#include <asm/msr.h>
#include <asm/processor.h>
#include <asm/spec_ctrl.h>
#include <asm/spec_ctrl_asm.h>

/* Cmdline controls for Xen's alternative blocks. */
static bool __initdata opt_msr_sc_pv = true;
static bool __initdata opt_msr_sc_hvm = true;
static bool __initdata opt_rsb_pv = true;
static bool __initdata opt_rsb_hvm = true;

/* Cmdline controls for Xen's speculative settings. */
static enum ind_thunk {
    THUNK_DEFAULT, /* Decide which thunk to use at boot time. */
    THUNK_NONE,    /* Missing compiler support for thunks. */

    THUNK_RETPOLINE,
    THUNK_LFENCE,
    THUNK_JMP,
} opt_thunk __initdata = THUNK_DEFAULT;
static int8_t __initdata opt_ibrs = -1;
bool __read_mostly opt_ibpb = true;

bool __initdata bsp_delay_spec_ctrl;
uint8_t __read_mostly default_xen_spec_ctrl;
uint8_t __read_mostly default_spec_ctrl_flags;

static int __init parse_bti(const char *s)
{
    const char *ss;
    int val, rc = 0;

    do {
        ss = strchr(s, ',');
        if ( !ss )
            ss = strchr(s, '\0');

        if ( !strncmp(s, "thunk=", 6) )
        {
            s += 6;

            if ( !strncmp(s, "retpoline", ss - s) )
                opt_thunk = THUNK_RETPOLINE;
            else if ( !strncmp(s, "lfence", ss - s) )
                opt_thunk = THUNK_LFENCE;
            else if ( !strncmp(s, "jmp", ss - s) )
                opt_thunk = THUNK_JMP;
            else
                rc = -EINVAL;
        }
        else if ( (val = parse_boolean("ibrs", s, ss)) >= 0 )
            opt_ibrs = val;
        else if ( (val = parse_boolean("ibpb", s, ss)) >= 0 )
            opt_ibpb = val;
        else if ( (val = parse_boolean("rsb_native", s, ss)) >= 0 )
            opt_rsb_pv = val;
        else if ( (val = parse_boolean("rsb_vmexit", s, ss)) >= 0 )
            opt_rsb_hvm = val;
        else
            rc = -EINVAL;

        s = ss + 1;
    } while ( *ss );

    return rc;
}
custom_param("bti", parse_bti);

static int __init parse_spec_ctrl(const char *s)
{
    const char *ss;
    int val, rc = 0;

    do {
        ss = strchr(s, ',');
        if ( !ss )
            ss = strchr(s, '\0');

        /* Global and Xen-wide disable. */
        val = parse_bool(s);
        if ( !val )
        {
            opt_msr_sc_pv = false;
            opt_msr_sc_hvm = false;

        disable_common:
            opt_rsb_pv = false;
            opt_rsb_hvm = false;

            opt_thunk = THUNK_JMP;
            opt_ibrs = 0;
            opt_ibpb = false;
        }
        else if ( val > 0 )
            rc = -EINVAL;
        else if ( (val = parse_boolean("xen", s, ss)) >= 0 )
        {
            if ( !val )
                goto disable_common;

            rc = -EINVAL;
        }

        /* Xen's alternative blocks. */
        else if ( (val = parse_boolean("pv", s, ss)) >= 0 )
        {
            opt_msr_sc_pv = val;
            opt_rsb_pv = val;
        }
        else if ( (val = parse_boolean("hvm", s, ss)) >= 0 )
        {
            opt_msr_sc_hvm = val;
            opt_rsb_hvm = val;
        }
        else if ( (val = parse_boolean("msr-sc", s, ss)) >= 0 )
        {
            opt_msr_sc_pv = val;
            opt_msr_sc_hvm = val;
        }
        else if ( (val = parse_boolean("rsb", s, ss)) >= 0 )
        {
            opt_rsb_pv = val;
            opt_rsb_hvm = val;
        }

        /* Xen's speculative sidechannel mitigation settings. */
        else if ( !strncmp(s, "bti-thunk=", 10) )
        {
            s += 10;

            if ( !strncmp(s, "retpoline", ss - s) )
                opt_thunk = THUNK_RETPOLINE;
            else if ( !strncmp(s, "lfence", ss - s) )
                opt_thunk = THUNK_LFENCE;
            else if ( !strncmp(s, "jmp", ss - s) )
                opt_thunk = THUNK_JMP;
            else
                rc = -EINVAL;
        }
        else if ( (val = parse_boolean("ibrs", s, ss)) >= 0 )
            opt_ibrs = val;
        else if ( (val = parse_boolean("ibpb", s, ss)) >= 0 )
            opt_ibpb = val;
        else
            rc = -EINVAL;

        s = ss + 1;
    } while ( *ss );

    return rc;
}
custom_param("spec-ctrl", parse_spec_ctrl);

static void __init print_details(enum ind_thunk thunk, uint64_t caps)
{
    bool use_spec_ctrl = (boot_cpu_has(X86_FEATURE_SC_MSR_PV) ||
                          boot_cpu_has(X86_FEATURE_SC_MSR_HVM));
    unsigned int _7d0 = 0, e8b = 0, tmp;

    /* Collect diagnostics about available mitigations. */
    if ( boot_cpu_data.cpuid_level >= 7 )
        cpuid_count(7, 0, &tmp, &tmp, &tmp, &_7d0);
    if ( boot_cpu_data.extended_cpuid_level >= 0x80000008 )
        cpuid(0x80000008, &tmp, &e8b, &tmp, &tmp);

    printk("Speculative mitigation facilities:\n");

    /* Hardware features which pertain to speculative mitigations. */
    printk("  Hardware features:%s%s%s%s%s%s\n",
           (_7d0 & cpufeat_mask(X86_FEATURE_IBRSB)) ? " IBRS/IBPB" : "",
           (_7d0 & cpufeat_mask(X86_FEATURE_STIBP)) ? " STIBP"     : "",
           (e8b  & cpufeat_mask(X86_FEATURE_IBPB))  ? " IBPB"      : "",
           (caps & ARCH_CAPABILITIES_IBRS_ALL)      ? " IBRS_ALL"  : "",
           (caps & ARCH_CAPABILITIES_RDCL_NO)       ? " RDCL_NO"   : "",
           (caps & ARCH_CAPS_RSBA)                  ? " RSBA"      : "");

    /* Compiled-in support which pertains to BTI mitigations. */
    if ( IS_ENABLED(CONFIG_INDIRECT_THUNK) )
        printk("  Compiled-in support: INDIRECT_THUNK\n");

    /* Settings for Xen's protection, irrespective of guests. */
    printk("  Xen settings: BTI-Thunk %s, SPEC_CTRL: %s, Other:%s\n",
           thunk == THUNK_NONE      ? "N/A" :
           thunk == THUNK_RETPOLINE ? "RETPOLINE" :
           thunk == THUNK_LFENCE    ? "LFENCE" :
           thunk == THUNK_JMP       ? "JMP" : "?",
           !use_spec_ctrl                            ?  "No" :
           (default_xen_spec_ctrl & SPEC_CTRL_IBRS)  ?  "IBRS+" :  "IBRS-",
           opt_ibpb                                  ? " IBPB"  : "");

    /*
     * Alternatives blocks for protecting against and/or virtualising
     * mitigation support for guests.
     */
    printk("  Support for VMs: PV:%s%s%s, HVM:%s%s%s\n",
           (boot_cpu_has(X86_FEATURE_SC_MSR_PV) ||
            boot_cpu_has(X86_FEATURE_SC_RSB_PV))     ? ""               : " None",
           boot_cpu_has(X86_FEATURE_SC_MSR_PV)       ? " MSR_SPEC_CTRL" : "",
           boot_cpu_has(X86_FEATURE_SC_RSB_PV)       ? " RSB"           : "",
           (boot_cpu_has(X86_FEATURE_SC_MSR_HVM) ||
            boot_cpu_has(X86_FEATURE_SC_RSB_HVM))    ? ""               : " None",
           boot_cpu_has(X86_FEATURE_SC_MSR_HVM)      ? " MSR_SPEC_CTRL" : "",
           boot_cpu_has(X86_FEATURE_SC_RSB_HVM)      ? " RSB"           : "");

    printk("XPTI: %s\n",
           boot_cpu_has(X86_FEATURE_NO_XPTI) ? "disabled" : "enabled");
}

/* Calculate whether Retpoline is known-safe on this CPU. */
static bool __init retpoline_safe(uint64_t caps)
{
    unsigned int ucode_rev = this_cpu(ucode_cpu_info).cpu_sig.rev;

    if ( boot_cpu_data.x86_vendor == X86_VENDOR_AMD )
        return true;

    if ( boot_cpu_data.x86_vendor != X86_VENDOR_INTEL ||
         boot_cpu_data.x86 != 6 )
        return false;

    /*
     * RSBA may be set by a hypervisor to indicate that we may move to a
     * processor which isn't retpoline-safe.
     */
    if ( caps & ARCH_CAPS_RSBA )
        return false;

    switch ( boot_cpu_data.x86_model )
    {
    case 0x17: /* Penryn */
    case 0x1d: /* Dunnington */
    case 0x1e: /* Nehalem */
    case 0x1f: /* Auburndale / Havendale */
    case 0x1a: /* Nehalem EP */
    case 0x2e: /* Nehalem EX */
    case 0x25: /* Westmere */
    case 0x2c: /* Westmere EP */
    case 0x2f: /* Westmere EX */
    case 0x2a: /* SandyBridge */
    case 0x2d: /* SandyBridge EP/EX */
    case 0x3a: /* IvyBridge */
    case 0x3e: /* IvyBridge EP/EX */
    case 0x3c: /* Haswell */
    case 0x3f: /* Haswell EX/EP */
    case 0x45: /* Haswell D */
    case 0x46: /* Haswell H */
        return true;

        /*
         * Broadwell processors are retpoline-safe after specific microcode
         * versions.
         */
    case 0x3d: /* Broadwell */
        return ucode_rev >= 0x2a;
    case 0x47: /* Broadwell H */
        return ucode_rev >= 0x1d;
    case 0x4f: /* Broadwell EP/EX */
        return ucode_rev >= 0xb000021;
    case 0x56: /* Broadwell D */
        switch ( boot_cpu_data.x86_mask )
        {
        case 2:  return ucode_rev >= 0x15;
        case 3:  return ucode_rev >= 0x7000012;
        case 4:  return ucode_rev >= 0xf000011;
        case 5:  return ucode_rev >= 0xe000009;
        default:
            printk("Unrecognised CPU stepping %#x - assuming not reptpoline safe\n",
                   boot_cpu_data.x86_mask);
            return false;
        }
        break;

        /*
         * Skylake, Kabylake and Cannonlake processors are not retpoline-safe.
         */
    case 0x4e:
    case 0x55:
    case 0x5e:
    case 0x66:
    case 0x67:
    case 0x8e:
    case 0x9e:
        return false;

    default:
        printk("Unrecognised CPU model %#x - assuming not reptpoline safe\n",
               boot_cpu_data.x86_model);
        return false;
    }
}

void __init init_speculation_mitigations(void)
{
    enum ind_thunk thunk = THUNK_DEFAULT;
    bool use_spec_ctrl = false, ibrs = false;
    uint64_t caps = 0;

    if ( boot_cpu_has(X86_FEATURE_ARCH_CAPS) )
        rdmsrl(MSR_ARCH_CAPABILITIES, caps);

    /*
     * Has the user specified any custom BTI mitigations?  If so, follow their
     * instructions exactly and disable all heuristics.
     */
    if ( opt_thunk != THUNK_DEFAULT || opt_ibrs != -1 )
    {
        thunk = opt_thunk;
        ibrs  = !!opt_ibrs;
    }
    else
    {
        /*
         * Evaluate the safest Branch Target Injection mitigations to use.
         * First, begin with compiler-aided mitigations.
         */
        if ( IS_ENABLED(CONFIG_INDIRECT_THUNK) )
        {
            /*
             * AMD's recommended mitigation is to set lfence as being dispatch
             * serialising, and to use IND_THUNK_LFENCE.
             */
            if ( cpu_has_lfence_dispatch )
                thunk = THUNK_LFENCE;
            /*
             * On Intel hardware, we'd like to use retpoline in preference to
             * IBRS, but only if it is safe on this hardware.
             */
            else if ( retpoline_safe(caps) )
                thunk = THUNK_RETPOLINE;
            else if ( boot_cpu_has(X86_FEATURE_IBRSB) )
                ibrs = true;
        }
        /* Without compiler thunk support, use IBRS if available. */
        else if ( boot_cpu_has(X86_FEATURE_IBRSB) )
            ibrs = true;
    }

    /*
     * Supplimentary minor adjustments.  Without compiler support, there are
     * no thunks.
     */
    if ( !IS_ENABLED(CONFIG_INDIRECT_THUNK) )
        thunk = THUNK_NONE;

    /*
     * If IBRS is in use and thunks are compiled in, there is no point
     * suffering extra overhead.  Switch to the least-overhead thunk.
     */
    if ( ibrs && thunk == THUNK_DEFAULT )
        thunk = THUNK_JMP;

    /*
     * If there are still no thunk preferences, the compiled default is
     * actually retpoline, and it is better than nothing.
     */
    if ( thunk == THUNK_DEFAULT )
        thunk = THUNK_RETPOLINE;

    /* Apply the chosen settings. */
    if ( thunk == THUNK_LFENCE )
        setup_force_cpu_cap(X86_FEATURE_IND_THUNK_LFENCE);
    else if ( thunk == THUNK_JMP )
        setup_force_cpu_cap(X86_FEATURE_IND_THUNK_JMP);

    /*
     * If we are on hardware supporting MSR_SPEC_CTRL, see about setting up
     * the alternatives blocks so we can virtualise support for guests.
     */
    if ( boot_cpu_has(X86_FEATURE_IBRSB) )
    {
        if ( opt_msr_sc_pv )
        {
            use_spec_ctrl = true;
            setup_force_cpu_cap(X86_FEATURE_SC_MSR_PV);
        }

        if ( opt_msr_sc_hvm )
        {
            use_spec_ctrl = true;
            setup_force_cpu_cap(X86_FEATURE_SC_MSR_HVM);
        }

        if ( use_spec_ctrl )
        {
            if ( ibrs )
                default_xen_spec_ctrl |= SPEC_CTRL_IBRS;

            default_spec_ctrl_flags |= SCF_ist_wrmsr;
        }
    }

    /*
     * PV guests can poison the RSB to any virtual address from which
     * they can execute a call instruction.  This is necessarily outside
     * of the Xen supervisor mappings.
     *
     * With SMEP enabled, the processor won't speculate into user mappings.
     * Therefore, in this case, we don't need to worry about poisoned entries
     * from 64bit PV guests.
     *
     * 32bit PV guest kernels run in ring 1, so use supervisor mappings.
     * If a processors speculates to 32bit PV guest kernel mappings, it is
     * speculating in 64bit supervisor mode, and can leak data.
     */
    if ( opt_rsb_pv )
    {
        setup_force_cpu_cap(X86_FEATURE_SC_RSB_PV);
        default_spec_ctrl_flags |= SCF_ist_rsb;
    }

    /*
     * HVM guests can always poison the RSB to point at Xen supervisor
     * mappings.
     */
    if ( opt_rsb_hvm )
        setup_force_cpu_cap(X86_FEATURE_SC_RSB_HVM);

    /* Check we have hardware IBPB support before using it... */
    if ( !boot_cpu_has(X86_FEATURE_IBRSB) && !boot_cpu_has(X86_FEATURE_IBPB) )
        opt_ibpb = false;

    /* (Re)init BSP state now that default_spec_ctrl_flags has been calculated. */
    init_shadow_spec_ctrl_state();

    /* If Xen is using any MSR_SPEC_CTRL settings, adjust the idle path. */
    if ( default_xen_spec_ctrl )
        setup_force_cpu_cap(X86_FEATURE_SC_MSR_IDLE);

    print_details(thunk, caps);

    /*
     * If MSR_SPEC_CTRL is available, apply Xen's default setting and discard
     * any firmware settings.  For performance reasons, when safe to do so, we
     * delay applying non-zero settings until after dom0 has been constructed.
     *
     * "when safe to do so" is based on whether we are virtualised.  A native
     * boot won't have any other code running in a position to mount an
     * attack.
     */
    if ( boot_cpu_has(X86_FEATURE_IBRSB) )
    {
        bsp_delay_spec_ctrl = !cpu_has_hypervisor && default_xen_spec_ctrl;

        /*
         * If delaying MSR_SPEC_CTRL setup, use the same mechanism as
         * spec_ctrl_enter_idle(), by using a shadow value of zero.
         */
        if ( bsp_delay_spec_ctrl )
        {
            struct cpu_info *info = get_cpu_info();

            info->shadow_spec_ctrl = 0;
            barrier();
            info->spec_ctrl_flags |= SCF_use_shadow;
            barrier();
        }

        wrmsrl(MSR_SPEC_CTRL, bsp_delay_spec_ctrl ? 0 : default_xen_spec_ctrl);
    }
}

static void __init __maybe_unused build_assertions(void)
{
    /* The optimised assembly relies on this alias. */
    BUILD_BUG_ON(SCF_use_shadow != 1);
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
