use warnings;
use Getopt::Std;
use File::Basename;
use File::Spec;
use IPC::Cmd;
use POSIX;
use Config;
use Carp;

# These control our behavior.
my $DRYRUN;
my $VERSION;
my $CCVENDOR;
my $CCVER;
my $CL_ARCH;
my $GCC_BITS;
my $GCC_ARCH;

# Some environment variables; they will affect Configure
my @cc_version =
    (
     clang => sub {
         return undef unless IPC::Cmd::can_run("$CROSS_COMPILE$CC");
         my $v = `$CROSS_COMPILE$CC -v 2>&1`;
         $v =~ m/(?:(?:clang|LLVM) version|.*based on LLVM)\s+([0-9]+\.[0-9]+)/;
         return $1;
     },
     gnu => sub {
         return undef unless IPC::Cmd::can_run("$CROSS_COMPILE$CC");
         my $nul = File::Spec->devnull();
         my $v = `$CROSS_COMPILE$CC -dumpversion 2> $nul`;
         # Strip off whatever prefix egcs prepends the number with.
         # Hopefully, this will work for any future prefixes as well.
         $v =~ s/^[a-zA-Z]*\-//;
         return $v;
    [ 'CYGWIN.*',                   '${MACHINE}-pc-cygwin' ],
    [ 'vxworks.*',                  '${MACHINE}-whatever-vxworks' ],

    # The MACHINE part of the array POSIX::uname() returns on VMS isn't
    # worth the bits wasted on it.  It's better, then, to rely on perl's
    # %Config, which has a trustworthy item 'archname', especially since
    # VMS installation aren't multiarch (yet)
    [ 'OpenVMS:.*',                 "$Config{archname}-whatever-OpenVMS" ],

    # Note: there's also NEO and NSR, but they are old and unsupported
    [ 'NONSTOP_KERNEL:.*:NSE-.*?',  'nse-tandem-nsk${RELEASE}' ],
    [ 'NONSTOP_KERNEL:.*:NSV-.*?',  'nsv-tandem-nsk${RELEASE}' ],
    [ 'NONSTOP_KERNEL:.*:NSX-.*?',  'nsx-tandem-nsk${RELEASE}' ],
                $CCVER = $v;
            }
        }

        # 'Windows NT' is the system name according to POSIX::uname()!
        if ( $SYSTEM eq "Windows NT" ) {
            # favor vendor cl over gcc
            if (IPC::Cmd::can_run('cl')) {
                $CC = 'cl';
                $CCVENDOR = ''; # Determine later
                $CCVER = 0;

                my $v = `cl 2>&1`;
                if ( $v =~ /Microsoft .* Version ([0-9\.]+) for (x86|x64|ARM|ia64)/ ) {
                    $CCVER = $1;
                    $CL_ARCH = $2;
                }
            }
        }
    }

    # If no C compiler has been determined at this point, we die.  Hard.
    die <<_____
            } else {
                $config{disable} = [ 'asm' ];
            }
            return { %config };
        }
      ],

      # Windows values found by looking at Perl 5's win32/win32.c
      [ '(amd64|ia64|x86|ARM)-.*?-Windows NT',
        sub {
            # If we determined the arch by asking cl, take that value,
            # otherwise the SYSTEM we got from from POSIX::uname().
            my $arch = $CL_ARCH // $1;
            my $config;

            if ($arch) {
                $config = { 'amd64' => { target => 'VC-WIN64A'    },
                            'ia64'  => { target => 'VC-WIN64I'    },
                            'x86'   => { target => 'VC-WIN32'     },
                            'x64'   => { target => 'VC-WIN64A'    },
                            'ARM'   => { target => 'VC-WIN64-ARM' },
                          } -> {$arch};
                die <<_____ unless defined $config;
ERROR
I do not know how to handle ${arch}.
_____
            }
            die <<_____ unless defined $config;
ERROR
Could not figure out the architecture.
_____

            return $config;
        }
      ],

      # VMS values found by observation on existing machinery.
      [ 'VMS_AXP-.*?-OpenVMS',    { target => 'vms-alpha'  } ],
      [ 'VMS_IA64-.*?-OpenVMS',   { target => 'vms-ia64'   } ],
      [ 'VMS_x86_64-.*?-OpenVMS', { target => 'vms-x86_64' } ],

      # TODO: There are a few more choices among OpenSSL config targets, but
      # reaching them involves a bit more than just a host tripet.  Select
      # environment variables could do the job to cover for more granular