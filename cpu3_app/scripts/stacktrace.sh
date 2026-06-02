#!/bin/bash
#
# SPDX-License-Identifier: GPL-2.0
#

set -e

INPUT_FILE=$1

SYM_FILE="cpu3_app.sym"
ELF_FILE="cpu3_app.elf"
TOOLCHAIN_ADDR2LINE="/opt/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-addr2line"

# Resolve a hex PC address to "func+offset/size" using the symbol file
function resolve_symbol()
{
	local pc_str="$1"
	cat ${SYM_FILE} | sort | awk -v pc="${pc_str}" '/\.(text|cpu3main)/ && $3 != "d" {
		if (strtonum(pc) < strtonum("0x"$1)) {
			exit
		}
		fname=$NF;
		fsize=strtonum("0x"$5);
		fpc=strtonum("0x"$1);
	}
	END {
		if (fname != "") printf("%s+0x%x/0x%x", fname, strtonum(pc) - fpc, fsize);
		else printf("???");
	}'
}

function unwind_addr()
{
	local ARG_FUNCADDR="$1"
	# Validate hex address with optional <hex>-<hex> relocation format
	if [[ ! "$ARG_FUNCADDR" =~ ^(0[xX])?[0-9a-fA-F]+(-(0[xX])?[0-9a-fA-F]+)?$ ]]; then
		echo "Error: Parameter '$1' is not a valid hex address." >&2
		exit 1
	fi

	local FUNCADDR="${ARG_FUNCADDR%%-*}"
	local RELOCOFF="${ARG_FUNCADDR#*-}"

	# Strip 0x prefix and lowercase
	FUNCADDR="${FUNCADDR,,}"
	FUNCADDR="${FUNCADDR#0x}"
	RELOCOFF="${RELOCOFF,,}"
	RELOCOFF="${RELOCOFF#0x}"

	# If reloc address is assigned and differs, subtract
	if [ "${FUNCADDR}" != "${RELOCOFF}" ] && [ -n "${RELOCOFF}" ]; then
		FUNCADDR=$(printf "%x" $((16#${FUNCADDR} - 16#${RELOCOFF})))
	fi

	echo
	sed -n "/${FUNCADDR}/p" ${SYM_FILE}
	${TOOLCHAIN_ADDR2LINE} -e ${ELF_FILE} ${FUNCADDR}
	exit 0
}

echo
if [ $# -eq 0 ]; then
	echo "Example:"
	echo "	./scripts/stacktrace.sh ./dump.txt"
	echo
	exit 1
elif [ ! -f ${INPUT_FILE} ]; then
	echo "ERROR: No ${INPUT_FILE}"
	exit 1
elif [ ! -f ${SYM_FILE} ]; then
	echo "ERROR: No ${SYM_FILE}"
	exit 1
fi

echo "SYMBOL File: ${SYM_FILE}"
echo
echo "Call trace:"
grep '\[< ' ${INPUT_FILE} | grep '>\]' | grep [PC,LR] | while read line
do
	echo -n " ${line}  "

	frame_pc_str=$(echo ${line} | awk '{ print "0x"$3 }')
	resolve_symbol "$frame_pc_str" | tr -d '\n'
	echo -n "      "

	func_path=$(unwind_addr "$frame_pc_str" | awk '{ print $1 }' | sed -n "/home/p")
	func_path=${func_path##*workspace/}
	echo ${func_path}
done
echo

echo "Stack:"
grep '\[< ' ${INPUT_FILE} | grep '>\]' | grep -v [PC,LR] | while read line
do
	echo -n "       ${line}  "

	frame_pc_str=$(echo ${line} | awk '{ print "0x"$2 }')
	resolve_symbol "$frame_pc_str"
	echo
done
echo
