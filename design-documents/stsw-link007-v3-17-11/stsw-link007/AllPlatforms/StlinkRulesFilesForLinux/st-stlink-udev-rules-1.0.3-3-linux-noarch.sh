#!/bin/sh
# This script was generated using Makeself 2.3.0

ORIG_UMASK=`umask`
if test "n" = n; then
    umask 077
fi

CRCsum="3738186949"
MD5="433fc770bf688bca1a1d30d1dac37dcf"
TMPROOT=${TMPDIR:=/tmp}
USER_PWD="$PWD"; export USER_PWD

label="STLink udev rules"
script="./setup.sh"
scriptargs="1.0.3-3"
licensetxt=""
helpheader=''
targetdir="makeself_dir_uZC7vQ"
filesizes="30720"
keep="y"
nooverwrite="n"
quiet="n"

print_cmd_arg=""
if type printf > /dev/null; then
    print_cmd="printf"
elif test -x /usr/ucb/echo; then
    print_cmd="/usr/ucb/echo"
else
    print_cmd="echo"
fi

unset CDPATH

MS_Printf()
{
    $print_cmd $print_cmd_arg "$1"
}

MS_PrintLicense()
{
  if test x"$licensetxt" != x; then
    echo "$licensetxt"
    while true
    do
      MS_Printf "Please type y to accept, n otherwise: "
      read yn
      if test x"$yn" = xn; then
        keep=n
	eval $finish; exit 1
        break;
      elif test x"$yn" = xy; then
        break;
      fi
    done
  fi
}

MS_diskspace()
{
	(
	if test -d /usr/xpg4/bin; then
		PATH=/usr/xpg4/bin:$PATH
	fi
	df -kP "$1" | tail -1 | awk '{ if ($4 ~ /%/) {print $3} else {print $4} }'
	)
}

MS_dd()
{
    blocks=`expr $3 / 1024`
    bytes=`expr $3 % 1024`
    dd if="$1" ibs=$2 skip=1 obs=1024 conv=sync 2> /dev/null | \
    { test $blocks -gt 0 && dd ibs=1024 obs=1024 count=$blocks ; \
      test $bytes  -gt 0 && dd ibs=1 obs=1024 count=$bytes ; } 2> /dev/null
}

MS_dd_Progress()
{
    if test x"$noprogress" = xy; then
        MS_dd $@
        return $?
    fi
    file="$1"
    offset=$2
    length=$3
    pos=0
    bsize=4194304
    while test $bsize -gt $length; do
        bsize=`expr $bsize / 4`
    done
    blocks=`expr $length / $bsize`
    bytes=`expr $length % $bsize`
    (
        dd ibs=$offset skip=1 2>/dev/null
        pos=`expr $pos \+ $bsize`
        MS_Printf "     0%% " 1>&2
        if test $blocks -gt 0; then
            while test $pos -le $length; do
                dd bs=$bsize count=1 2>/dev/null
                pcent=`expr $length / 100`
                pcent=`expr $pos / $pcent`
                if test $pcent -lt 100; then
                    MS_Printf "\b\b\b\b\b\b\b" 1>&2
                    if test $pcent -lt 10; then
                        MS_Printf "    $pcent%% " 1>&2
                    else
                        MS_Printf "   $pcent%% " 1>&2
                    fi
                fi
                pos=`expr $pos \+ $bsize`
            done
        fi
        if test $bytes -gt 0; then
            dd bs=$bytes count=1 2>/dev/null
        fi
        MS_Printf "\b\b\b\b\b\b\b" 1>&2
        MS_Printf " 100%%  " 1>&2
    ) < "$file"
}

MS_Help()
{
    cat << EOH >&2
${helpheader}Makeself version 2.3.0
 1) Getting help or info about $0 :
  $0 --help   Print this message
  $0 --info   Print embedded info : title, default target directory, embedded script ...
  $0 --lsm    Print embedded lsm entry (or no LSM)
  $0 --list   Print the list of files in the archive
  $0 --check  Checks integrity of the archive

 2) Running $0 :
  $0 [options] [--] [additional arguments to embedded script]
  with following options (in that order)
  --confirm             Ask before running embedded script
  --quiet		Do not print anything except error messages
  --noexec              Do not run embedded script
  --keep                Do not erase target directory after running
			the embedded script
  --noprogress          Do not show the progress during the decompression
  --nox11               Do not spawn an xterm
  --nochown             Do not give the extracted files to the current user
  --target dir          Extract directly to a target directory
                        directory path can be either absolute or relative
  --tar arg1 [arg2 ...] Access the contents of the archive through the tar command
  --                    Following arguments will be passed to the embedded script
EOH
}

MS_Check()
{
    OLD_PATH="$PATH"
    PATH=${GUESS_MD5_PATH:-"$OLD_PATH:/bin:/usr/bin:/sbin:/usr/local/ssl/bin:/usr/local/bin:/opt/openssl/bin"}
	MD5_ARG=""
    MD5_PATH=`exec <&- 2>&-; which md5sum || command -v md5sum || type md5sum`
    test -x "$MD5_PATH" || MD5_PATH=`exec <&- 2>&-; which md5 || command -v md5 || type md5`
	test -x "$MD5_PATH" || MD5_PATH=`exec <&- 2>&-; which digest || command -v digest || type digest`
    PATH="$OLD_PATH"

    if test x"$quiet" = xn; then
		MS_Printf "Verifying archive integrity..."
    fi
    offset=`head -n 525 "$1" | wc -c | tr -d " "`
    verb=$2
    i=1
    for s in $filesizes
    do
		crc=`echo $CRCsum | cut -d" " -f$i`
		if test -x "$MD5_PATH"; then
			if test x"`basename $MD5_PATH`" = xdigest; then
				MD5_ARG="-a md5"
			fi
			md5=`echo $MD5 | cut -d" " -f$i`
			if test x"$md5" = x00000000000000000000000000000000; then
				test x"$verb" = xy && echo " $1 does not contain an embedded MD5 checksum." >&2
			else
				md5sum=`MS_dd_Progress "$1" $offset $s | eval "$MD5_PATH $MD5_ARG" | cut -b-32`;
				if test x"$md5sum" != x"$md5"; then
					echo "Error in MD5 checksums: $md5sum is different from $md5" >&2
					exit 2
				else
					test x"$verb" = xy && MS_Printf " MD5 checksums are OK." >&2
				fi
				crc="0000000000"; verb=n
			fi
		fi
		if test x"$crc" = x0000000000; then
			test x"$verb" = xy && echo " $1 does not contain a CRC checksum." >&2
		else
			sum1=`MS_dd_Progress "$1" $offset $s | CMD_ENV=xpg4 cksum | awk '{print $1}'`
			if test x"$sum1" = x"$crc"; then
				test x"$verb" = xy && MS_Printf " CRC checksums are OK." >&2
			else
				echo "Error in checksums: $sum1 is different from $crc" >&2
				exit 2;
			fi
		fi
		i=`expr $i + 1`
		offset=`expr $offset + $s`
    done
    if test x"$quiet" = xn; then
		echo " All good."
    fi
}

UnTAR()
{
    if test x"$quiet" = xn; then
		tar $1vf - 2>&1 || { echo Extraction failed. > /dev/tty; kill -15 $$; }
    else

		tar $1f - 2>&1 || { echo Extraction failed. > /dev/tty; kill -15 $$; }
    fi
}

finish=true
xterm_loop=
noprogress=n
nox11=y
copy=none
ownership=y
verbose=n

initargs="$@"

while true
do
    case "$1" in
    -h | --help)
	MS_Help
	exit 0
	;;
    -q | --quiet)
	quiet=y
	noprogress=y
	shift
	;;
    --info)
	echo Identification: "$label"
	echo Target directory: "$targetdir"
	echo Uncompressed size: 44 KB
	echo Compression: none
	echo Date of packaging: Thu Jan 22 14:50:30 UTC 2026
	echo Built with Makeself version 2.3.0 on 
	echo Build command was: "/usr/bin/makeself \\
    \"--nocomp\" \\
    \"--nox11\" \\
    \"--notemp\" \\
    \"/tmp/makeself_dir_uZC7vQ\" \\
    \"st-stlink-udev-rules-1.0.3-3-linux-noarch.sh\" \\
    \"STLink udev rules\" \\
    \"./setup.sh\" \\
    \"1.0.3-3\""
	if test x"$script" != x; then
	    echo Script run after extraction:
	    echo "    " $script $scriptargs
	fi
	if test x"" = xcopy; then
		echo "Archive will copy itself to a temporary location"
	fi
	if test x"n" = xy; then
		echo "Root permissions required for extraction"
	fi
	if test x"y" = xy; then
	    echo "directory $targetdir is permanent"
	else
	    echo "$targetdir will be removed after extraction"
	fi
	exit 0
	;;
    --dumpconf)
	echo LABEL=\"$label\"
	echo SCRIPT=\"$script\"
	echo SCRIPTARGS=\"$scriptargs\"
	echo archdirname=\"makeself_dir_uZC7vQ\"
	echo KEEP=y
	echo NOOVERWRITE=n
	echo COMPRESS=none
	echo filesizes=\"$filesizes\"
	echo CRCsum=\"$CRCsum\"
	echo MD5sum=\"$MD5\"
	echo OLDUSIZE=44
	echo OLDSKIP=526
	exit 0
	;;
    --lsm)
cat << EOLSM
No LSM.
EOLSM
	exit 0
	;;
    --list)
	echo Target directory: $targetdir
	offset=`head -n 525 "$0" | wc -c | tr -d " "`
	for s in $filesizes
	do
	    MS_dd "$0" $offset $s | eval "cat" | UnTAR t
	    offset=`expr $offset + $s`
	done
	exit 0
	;;
	--tar)
	offset=`head -n 525 "$0" | wc -c | tr -d " "`
	arg1="$2"
    if ! shift 2; then MS_Help; exit 1; fi
	for s in $filesizes
	do
	    MS_dd "$0" $offset $s | eval "cat" | tar "$arg1" - "$@"
	    offset=`expr $offset + $s`
	done
	exit 0
	;;
    --check)
	MS_Check "$0" y
	exit 0
	;;
    --confirm)
	verbose=y
	shift
	;;
	--noexec)
	script=""
	shift
	;;
    --keep)
	keep=y
	shift
	;;
    --target)
	keep=y
	targetdir=${2:-.}
    if ! shift 2; then MS_Help; exit 1; fi
	;;
    --noprogress)
	noprogress=y
	shift
	;;
    --nox11)
	nox11=y
	shift
	;;
    --nochown)
	ownership=n
	shift
	;;
    --xwin)
	if test "n" = n; then
		finish="echo Press Return to close this window...; read junk"
	fi
	xterm_loop=1
	shift
	;;
    --phase2)
	copy=phase2
	shift
	;;
    --)
	shift
	break ;;
    -*)
	echo Unrecognized flag : "$1" >&2
	MS_Help
	exit 1
	;;
    *)
	break ;;
    esac
done

if test x"$quiet" = xy -a x"$verbose" = xy; then
	echo Cannot be verbose and quiet at the same time. >&2
	exit 1
fi

if test x"n" = xy -a `id -u` -ne 0; then
	echo "Administrative privileges required for this archive (use su or sudo)" >&2
	exit 1	
fi

if test x"$copy" \!= xphase2; then
    MS_PrintLicense
fi

case "$copy" in
copy)
    tmpdir=$TMPROOT/makeself.$RANDOM.`date +"%y%m%d%H%M%S"`.$$
    mkdir "$tmpdir" || {
	echo "Could not create temporary directory $tmpdir" >&2
	exit 1
    }
    SCRIPT_COPY="$tmpdir/makeself"
    echo "Copying to a temporary location..." >&2
    cp "$0" "$SCRIPT_COPY"
    chmod +x "$SCRIPT_COPY"
    cd "$TMPROOT"
    exec "$SCRIPT_COPY" --phase2 -- $initargs
    ;;
phase2)
    finish="$finish ; rm -rf `dirname $0`"
    ;;
esac

if test x"$nox11" = xn; then
    if tty -s; then                 # Do we have a terminal?
	:
    else
        if test x"$DISPLAY" != x -a x"$xterm_loop" = x; then  # No, but do we have X?
            if xset q > /dev/null 2>&1; then # Check for valid DISPLAY variable
                GUESS_XTERMS="xterm gnome-terminal rxvt dtterm eterm Eterm xfce4-terminal lxterminal kvt konsole aterm terminology"
                for a in $GUESS_XTERMS; do
                    if type $a >/dev/null 2>&1; then
                        XTERM=$a
                        break
                    fi
                done
                chmod a+x $0 || echo Please add execution rights on $0
                if test `echo "$0" | cut -c1` = "/"; then # Spawn a terminal!
                    exec $XTERM -title "$label" -e "$0" --xwin "$initargs"
                else
                    exec $XTERM -title "$label" -e "./$0" --xwin "$initargs"
                fi
            fi
        fi
    fi
fi

if test x"$targetdir" = x.; then
    tmpdir="."
else
    if test x"$keep" = xy; then
	if test x"$nooverwrite" = xy && test -d "$targetdir"; then
            echo "Target directory $targetdir already exists, aborting." >&2
            exit 1
	fi
	if test x"$quiet" = xn; then
	    echo "Creating directory $targetdir" >&2
	fi
	tmpdir="$targetdir"
	dashp="-p"
    else
	tmpdir="$TMPROOT/selfgz$$$RANDOM"
	dashp=""
    fi
    mkdir $dashp $tmpdir || {
	echo 'Cannot create target directory' $tmpdir >&2
	echo 'You should try option --target dir' >&2
	eval $finish
	exit 1
    }
fi

location="`pwd`"
if test x"$SETUP_NOCHECK" != x1; then
    MS_Check "$0"
fi
offset=`head -n 525 "$0" | wc -c | tr -d " "`

if test x"$verbose" = xy; then
	MS_Printf "About to extract 44 KB in $tmpdir ... Proceed ? [Y/n] "
	read yn
	if test x"$yn" = xn; then
		eval $finish; exit 1
	fi
fi

if test x"$quiet" = xn; then
	MS_Printf "Uncompressing $label"
fi
res=3
if test x"$keep" = xn; then
    trap 'echo Signal caught, cleaning up >&2; cd $TMPROOT; /bin/rm -rf $tmpdir; eval $finish; exit 15' 1 2 3 15
fi

leftspace=`MS_diskspace $tmpdir`
if test -n "$leftspace"; then
    if test "$leftspace" -lt 44; then
        echo
        echo "Not enough space left in "`dirname $tmpdir`" ($leftspace KB) to decompress $0 (44 KB)" >&2
        if test x"$keep" = xn; then
            echo "Consider setting TMPDIR to a directory with more free space."
        fi
        eval $finish; exit 1
    fi
fi

for s in $filesizes
do
    if MS_dd_Progress "$0" $offset $s | eval "cat" | ( cd "$tmpdir"; umask $ORIG_UMASK ; UnTAR xp ) 1>/dev/null; then
		if test x"$ownership" = xy; then
			(PATH=/usr/xpg4/bin:$PATH; cd "$tmpdir"; chown -R `id -u` .;  chgrp -R `id -g` .)
		fi
    else
		echo >&2
		echo "Unable to decompress $0" >&2
		eval $finish; exit 1
    fi
    offset=`expr $offset + $s`
done
if test x"$quiet" = xn; then
	echo
fi

cd "$tmpdir"
res=0
if test x"$script" != x; then
    if test x"$verbose" = x"y"; then
		MS_Printf "OK to execute: $script $scriptargs $* ? [Y/n] "
		read yn
		if test x"$yn" = x -o x"$yn" = xy -o x"$yn" = xY; then
			eval "\"$script\" $scriptargs \"\$@\""; res=$?;
		fi
    else
		eval "\"$script\" $scriptargs \"\$@\""; res=$?
    fi
    if test "$res" -ne 0; then
		test x"$verbose" = xy && echo "The program '$script' returned an error code ($res)" >&2
    fi
fi
if test x"$keep" = xn; then
    cd $TMPROOT
    /bin/rm -rf $tmpdir
fi
eval $finish; exit $res
./                                                                                                  0000700 0007127 0127032 00000000000 15134434466 011005  5                                                                                                    ustar   chavagna                        gnbmcd                                                                                                                                                                                                                 ./setup.sh                                                                                          0000755 0007127 0127032 00000002173 15134434466 012521  0                                                                                                    ustar   chavagna                        gnbmcd                                                                                                                                                                                                                 #!/bin/bash

thisdir=$(readlink -m $(dirname $0))

version=$1


if [ $(id -u) != 0 ]; then
	echo >&2 "Must be root to install this package. Aborting."
	exit 1
fi

if [ -z "$version" ] ; then
	echo >&2 "missing version arg. Aborting installation."
	exit 1
fi

# Ask user to agree on license
bash $thisdir/prompt_linux_license.sh
if [ $? -ne 0 ]
then
	$thisdir/cleanup.sh
	exit 1
fi


rules_path=/etc/udev/rules.d/
set junk ${rules_path}*
shift
first_rule=$1

if [ -f "$first_rule" ] ; then
	# If there are already installed files, check for stamps (ST_PKG_VERSION)
	# Check all versions including the one to be installed and see which is the latest (sort -u|tail -1)
	latest_version=$(
		(
			echo $version
			awk -F '[ \t=]+' '/ST_PKG_VERSION/{print $3}' ${rules_path}*
		) |sort -uV | tail -1
	)

	# latest_version should be our candidate for installation
	if [ $latest_version != "$version" ]; then
		echo >&2 "Not overwriting version $latest_version with an older one ($version)"
		exit 0
	fi
fi

# Finally do the installation
install -D -o root -g root -m 644 -t $rules_path $thisdir/*.rules

echo >&2 "Package installed."
$thisdir/cleanup.sh
                                                                                                                                                                                                                                                                                                                                                                                                     ./49-stlinkv1.rules                                                                                 0000644 0007127 0127032 00000000365 15134434466 014104  0                                                                                                    ustar   chavagna                        gnbmcd                                                                                                                                                                                                                 # stm32 discovery boards, with onboard st/linkv1
# ie, STM32VL.

SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3744", \
    MODE="660", GROUP="plugdev", TAG+="uaccess", ENV{ID_MM_DEVICE_IGNORE}="1", \
    SYMLINK+="stlinkv1_%n"
                                                                                                                                                                                                                                                                           ./cleanup.sh                                                                                        0000755 0007127 0127032 00000000526 15134434466 013010  0                                                                                                    ustar   chavagna                        gnbmcd                                                                                                                                                                                                                 #!/bin/bash

thisdir=$(readlink -m $(dirname $0))

cd $thisdir

# Selective cleanup as self extract may have been
# done in a user-created dir.

# Remove known objects
for item in $(cat pkg_rootdir_content.txt) root
do
        rm -rf $item
done

# Attempt to remove dir only if it's empty
if [ -z "$(ls -A)" ]; then
        rmdir $thisdir
fi
                                                                                                                                                                          ./49-stlinkv2-1.rules                                                                               0000644 0007127 0127032 00000000667 15134434466 014250  0                                                                                                    ustar   chavagna                        gnbmcd                                                                                                                                                                                                                 # stm32 nucleo boards, with onboard st/linkv2-1
# ie, STM32F0, STM32F4.

SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374b", \
    MODE="660", GROUP="plugdev", TAG+="uaccess", ENV{ID_MM_DEVICE_IGNORE}="1", \
    SYMLINK+="stlinkv2-1_%n"

SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3752", \
    MODE="660", GROUP="plugdev", TAG+="uaccess", ENV{ID_MM_DEVICE_IGNORE}="1", \
    SYMLINK+="stlinkv2-1_%n"

                                                                         ./prompt_linux_license.sh                                                                           0000644 0007127 0127032 00000022322 15134434466 015616  0                                                                                                    ustar   chavagna                        gnbmcd                                                                                                                                                                                                                 #!/bin/bash

if [ "$LICENSE_ALREADY_ACCEPTED" ] ; then
	exit 0
fi

display_license() {
cat << EOF
STMicroelectronics Software License Agreement

SLA0048 Rev5/October 2025

Please indicate your acceptance or NON-acceptance by selecting "I ACCEPT" or "I DO NOT ACCEPT" as indicated below in the media.

BY CLICKING ON THE "I ACCEPT" BUTTON OR BY UNZIPPING, INSTALLING, COPYING, DOWNLOADING, ACCESSING OR OTHERWISE USING THIS SOFTWARE PACKAGE OR ANY PART THEREOF, INCLUDING ANY RELATED DOCUMENTATION (collectively the "SOFTWARE PACKAGE") FROM STMICROELECTRONICS INTERNATIONAL N.V, SWISS BRANCH AND/OR ITS AFFILIATED COMPANIES (collectively "STMICROELECTRONICS"), YOU (hereinafter referred also to as "THE RECIPIENT"), ON BEHALF OF YOURSELF, OR ON BEHALF OF ANY ENTITY BY WHICH YOU ARE EMPLOYED AND/OR ENGAGED, AGREE TO BE BOUND BY THIS AGREEMENT.

Under STMICROELECTRONICS' intellectual property rights and subject to applicable licensing terms for any third-party software incorporated in the SOFTWARE PACKAGE and applicable Open Source Terms (as defined here below), the redistribution, reproduction and use in source and binary forms of the SOFTWARE PACKAGE or any part thereof, with or without modification, are permitted provided that the following conditions are met:
1. Redistribution of source code (modified or not) must retain any copyright notice accompanying the SOFTWARE PACKAGE, this list of conditions and the disclaimer below.
2. Redistributions in binary form, except as embedded into a processing unit device manufactured by or for STMicroelectronics or a software update for any such device, must reproduce the accompanying copyright notice, this list of conditions and the below disclaimer in capital type, in the documentation and/or other materials provided with the distribution.
3. Neither the name of STMicroelectronics nor the names of other contributors to the SOFTWARE PACKAGE may be used to endorse or promote products derived from the SOFTWARE PACKAGE or part thereof without specific written permission.
4. The SOFTWARE PACKAGE or any part thereof, including modifications and/or derivative works of the SOFTWARE PACKAGE, must be used and execute solely and exclusively on or in combination with a processing unit device manufactured by or for STMicroelectronics.
5. No use, reproduction or redistribution of the SOFTWARE PACKAGE partially or totally may be done in any manner that would subject the SOFTWARE PACKAGE to any Open Source Terms (as defined below).
6. Some portion of the SOFTWARE PACKAGE may contain software subject to Open Source Terms (as defined below) applicable for each such portion ("Open Source Software"), as further specified in the SOFTWARE PACKAGE. Such Open Source Software is supplied under the applicable Open Source Terms and is not subject to the terms and conditions of this AGREEMENT. "Open Source Terms" shall mean any open source license which requires as part of distribution of software that the source code of such software is distributed therewith or otherwise made available, or open source license that substantially complies with the Open Source definition specified at www.opensource.org and any other comparable open source license such as for example GNU General Public License (GPL), Eclipse Public License (EPL), Apache Software License, BSD license and MIT license.
7. The SOFTWARE PACKAGE may also include third party software as expressly specified in the SOFTWARE PACKAGE subject to specific license terms from such third parties. Such third-party software is supplied under such specific license terms and is not subject to the terms and conditions of this AGREEMENT. By installing copying, downloading, accessing or otherwise using the SOFTWARE PACKAGE, the RECIPIENT agrees to be bound by such license terms with regard to such third-party software.
8. STMicroelectronics has no obligation to provide any maintenance, support or updates for the SOFTWARE PACKAGE.
9. The SOFTWARE PACKAGE is and will remain the exclusive property of STMicroelectronics and its licensors. The RECIPIENT will not take any action that jeopardizes STMicroelectronics and its licensors' proprietary rights or acquire any rights in the SOFTWARE PACKAGE, except the limited rights specified hereunder.
10. The RECIPIENT shall comply with all applicable laws and regulations affecting the use of the SOFTWARE PACKAGE or any part thereof including any applicable export control law or regulation.
11. Redistribution and use of the SOFTWARE PACKAGE partially or any part thereof other than as permitted under this AGREEMENT is void and will automatically terminate RECIPIENT's rights under this AGREEMENT.
12. The RECIPIENT shall be solely liable to determine and verify that the SOFTWARE PACKAGE is fit for the RECIPIENT intended use, environment or application and comply with all regulatory, safety and security related requirements concerning any use.

THE SOFTWARE PACKAGE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THE SOFTWARE PACKAGE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
EXCEPT AS EXPRESSLY PERMITTED HEREUNDER AND SUBJECT TO THE APPLICABLE LICENSING TERMS FOR ANY THIRD-PARTY SOFTWARE INCORPORATED IN THE SOFTWARE PACKAGE AND OPEN SOURCE TERMS AS APPLICABLE, NO LICENSE OR OTHER RIGHTS, WHETHER EXPRESS OR IMPLIED, ARE GRANTED UNDER ANY PATENT OR OTHER INTELLECTUAL PROPERTY RIGHTS OF STMICROELECTRONICS OR ANY THIRD PARTY.
EOF
}

# Make sure we use bash (#! may be overriden by caller script)
if [ "$(ps -o comm h -p $$)" != 'bash' ]
then
	exec /bin/bash "$0" "$@"
fi

export -f display_license

# Prompt user for license acceptance.
# Depending on options and environment, choose proper display tool.
# As terminal mode may not be detected when run from a script,
#   --force-console is here for automation purpose when testing. (ie. using expect)

set -e

box_title="STM32CubeIDE - License Agreement"

terminal_prompt() {
	rc_file=$1
	local rc
	local_dir=$0
	dir_path=$(echo "$local_dir" | grep -oE '.*\/')
	file_gen_install="$dir_path"*.tar.gz
	# terminal_width=$(tput cols)
	# line=$(printf '%*s' "$terminal_width" '' | tr ' ' '=')

	if ls $file_gen_install 1> /dev/null 2>&1; then
		echo " "
		echo " To be able to have the tool fully functioning, please install the Libncurses5 manually "
		echo " "
	else
		echo " "
	fi



	typeset -l answer
	display_license | more
	echo
	read -p "I ACCEPT (y) / I DO NOT ACCEPT (N) [N/y] " answer
	if [ "$answer" = "y" ]; then
		# License accepted
		rc=0
		echo "License accepted."
	else
		# License not accepted
		rc=1
		echo "*** License NOT accepted. Not installing software. Hit return to exit."
		read
	fi

	# If exit code cannot be captured by caller, use this temp file
	if [ "$rc_file" ]
	then
		echo $rc > $rc_file
	fi

	exit $rc
}
export -f terminal_prompt

# Special treatment for RPM
if [[ ${BASH_SOURCE[0]} =~ '/var/tmp/rpm-tmp.' ]]; then
	if [ "$INTERACTIVE" = FALSE ] ; then
		# If not interactive and DISPLAY is not set (X11 installer seems to not propagate this variable)
		# then force it to :0
		export DISPLAY=${DISPLAY:-:0}
		# If this fails, then installation fails and user does not know it but what else can we do?
	else
		# Restore stdin as rpm installer closes it before running scriptlets.
		exec 0</dev/tty
	fi
fi

if [ -t 0 -o "$STM_FORCE_CONSOLE" ]
then
	# Terminal detected or wanted
	terminal_prompt

	# Unreached
	echo >&2 "Bug in $0 (terminal_prompt)"
	exit 3
fi

# No terminal
if [ -z "$DISPLAY" ]
then
	echo >&2 "DISPLAY not set. Cannot display license. Aborting."
	exit 2
fi


# Find first available X11 tool
dialog_tools="zenity xterm"
for tool in $dialog_tools
do
	if ( type >/dev/null -f $tool )
	then
		dialog=$tool
		break
	fi
done

case $dialog in
xterm)
	# Use terminal mode in an xterm

	# Workaround as xterm does not return "-e command" exit code
	exit_code_tmp_file=$(mktemp)
	xterm -title "$box_title" -ls -geometry 115x40 -sb -sl 1000 -e "terminal_prompt $exit_code_tmp_file"
	rc=$(cat $exit_code_tmp_file)
	rm $exit_code_tmp_file
	exit $rc
	;;
zenity)
	# Little trick below as default button of zenity is 'ok' and we want it to be 'cancel'.
	# So just swap buttons labels and use reverse condition for acceptance.
	display_license | zenity \
		--text-info \
		--title="$box_title" \
		--width=650 --height=500 \
		--cancel-label "I ACCEPT" \
		--ok-label "I DO NOT ACCEPT" \
		|| exit 0 # Accepted

	# Not accepted
	zenity \
		--error \
		--title="$box_title" \
		--text "License NOT accepted. Not installing software."
	exit 1
	;;
*)
	echo >&2 "No dialog tool found to display license. Aborting."
	exit 2
esac

# Should be unreached
echo >&2 "No way to display license. Aborting."
exit 3
                                                                                                                                                                                                                                                                                                              ./49-stlinkv3.rules                                                                                 0000644 0007127 0127032 00000002530 15134434466 014102  0                                                                                                    ustar   chavagna                        gnbmcd                                                                                                                                                                                                                 # stlink-v3 boards (standalone and embedded) in usbloader mode and standard (debug) mode

SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374d", \
    MODE="660", GROUP="plugdev", TAG+="uaccess", ENV{ID_MM_DEVICE_IGNORE}="1", \
    SYMLINK+="stlinkv3loader_%n"

SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374e", \
    MODE="660", GROUP="plugdev", TAG+="uaccess", ENV{ID_MM_DEVICE_IGNORE}="1", \
    SYMLINK+="stlinkv3_%n"

SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374f", \
    MODE="660", GROUP="plugdev", TAG+="uaccess", ENV{ID_MM_DEVICE_IGNORE}="1", \
    SYMLINK+="stlinkv3_%n"

SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3753", \
    MODE="660", GROUP="plugdev", TAG+="uaccess", ENV{ID_MM_DEVICE_IGNORE}="1", \
    SYMLINK+="stlinkv3_%n"

SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3754", \
    MODE="660", GROUP="plugdev", TAG+="uaccess", ENV{ID_MM_DEVICE_IGNORE}="1", \
    SYMLINK+="stlinkv3_%n"

SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3755", \
    MODE="660", GROUP="plugdev", TAG+="uaccess", ENV{ID_MM_DEVICE_IGNORE}="1", \
    SYMLINK+="stlinkv3loader_%n"

SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3757", \
    MODE="660", GROUP="plugdev", TAG+="uaccess", ENV{ID_MM_DEVICE_IGNORE}="1", \
    SYMLINK+="stlinkv3_%n"
                                                                                                                                                                        ./49-stlinkv2.rules                                                                                 0000644 0007127 0127032 00000000375 15134434466 014106  0                                                                                                    ustar   chavagna                        gnbmcd                                                                                                                                                                                                                 # stm32 discovery boards, with onboard st/linkv2
# ie, STM32L, STM32F4.

SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3748", \
    MODE="660", GROUP="plugdev", TAG+="uaccess", ENV{ID_MM_DEVICE_IGNORE}="1", \
    SYMLINK+="stlinkv2_%n"
                                                                                                                                                                                                                                                                   ./pkg_rootdir_content.txt                                                                           0000644 0007127 0127032 00000000216 15134434466 015634  0                                                                                                    ustar   chavagna                        gnbmcd                                                                                                                                                                                                                 49-stlinkv1.rules
49-stlinkv2-1.rules
49-stlinkv2.rules
49-stlinkv3.rules
cleanup.sh
pkg_rootdir_content.txt
prompt_linux_license.sh
setup.sh
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  