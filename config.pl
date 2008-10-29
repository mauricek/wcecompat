#!/usr/local/bin/perl -w

### Input / Variable Parsing ###

die '%OSVERSION% is not defined, see README.txt' if (!defined($ENV{'OSVERSION'}));
die '%TARGETCPU% is not defined, see README.txt' if (!defined($ENV{'TARGETCPU'}));

$wince_version = uc($ENV{'OSVERSION'});
die '%OSVERSION% is not properly set, see README.txt' if ($wince_version !~ /^WCE([1-9])([0-9]{2})$/);

$wince_major_version = $1;
$wince_minor_version=$2;

die '%PLATFORM% is not defined, see README.txt' if ($wince_major_version < 5 && !defined($ENV{'PLATFORM'}));

print "windows ce version:".$wince_version."\n";
print "major:".$wince_major_version."\n";
print "minor:".$wince_minor_version."\n";

$target_cpu = uc($ENV{'TARGETCPU'});
print "Compiling for Architecture:".$target_cpu."\n";


### Setting up variables  ###

# first set the version flags for compiling and linking
$compile_flags = " -D_WIN32_WCE=0x".$wince_major_version.$wince_minor_version;
$compile_flags.= " -DUNDER_CE=$wince_major_version$wince_minor_version";
$link_flags = "/SUBSYSTEM:windowsce,$wince_major_version.$wince_minor_version";
$link_target_cpu = $target_cpu;

# each platform has its own compiler and linker flags
if ($target_cpu eq "X86") {
		$compile_flags.= " -Dx86 -D_X86_";
		$link_flags.= " /NODEFAULTLIB:oldnames.lib coredll.lib corelibc.lib";
		}
elsif ($target_cpu eq "X86EM" || $target_cpu eq "X86EMNSET CFG=NONE") {
		$compile_flags.= " -Dx86 -D_X86_ -D_WIN32_WCE_EMULATION";
		$link_target_cpu = "X86";
		}
elsif ($target_cpu eq "SH3") {
		$compile_flags.= " -DSH3 -D_SH3_ -DSHx";
		}
elsif ($target_cpu eq "SH4") {
		$compile_flags.= " -DSH4 -D_SH4_ -DSHx";
		}
elsif ($target_cpu eq "R4100" || $target_cpu eq "R4111" || $target_cpu eq "R4300" || $target_cpu eq "MIPSII" || $target_cpu eq "MIPSIV") {
		$compile_flags.= " -DMIPS -D_MIPS_ -DMIPS_R4000";
		}
elsif ($target_cpu eq "ARMV4" || $target_cpu eq "ARMV4T" || $target_cpu eq "ARMV4I") {
		$compile_flags.= " -DARM -D_ARM_ -D_M_ARM -D$target_cpu -D_$target_cpu\_";
		if ($wince_major_version >= 5) {
			$link_target_cpu = "THUMB";
		}
		else {
			$link_target_cpu = "ARM";
		}
}

# We need to set the machine type for the link flag
if ($link_target_cpu eq "X86" && $wince_major_version < 5) {
	$link_target_cpu = "IX86";
} elsif ($link_target_cpu eq "MIPSII" || $link_target_cpu eq "MIPSIV") {
	$link_target_cpu = "MIPS";
}
$link_flags.= " /MACHINE:".$link_target_cpu;

print "Compile:".$compile_flags."\n";
print "Link:".$link_flags."\n";

# And finally we write out the configuration
open (FILEHANDLE, '>wceconfig.mak');
#WCEVERSION=200
#WCELDVERSION=2.00

print FILEHANDLE 'WCEVERSION='.$wince_major_version.$wince_minor_version."\n";
print FILEHANDLE 'WCELDVERSION='.$wince_major_version.'.'.$wince_minor_version."\n";
print FILEHANDLE 'WCEPLATFORM=foobar'."\n";
print FILEHANDLE 'WCETARGETDEFS='.$compile_flags."\n";
print FILEHANDLE 'LFLAGS='.$link_flags."\n";

close(FILEHANDLE);


