#!/bin/bash

# $1 - string
# out - left trimmed string
function StrLTrim ()
{
    echo "$1" | sed "s,^[	 ]*,,g"
}

# $1 - string
# out - right trimmed string
function StrRTrim ()
{
    echo "$1" | sed "s,[	 ]*\$,,g"
}

# $1 - string
# out - trimmed string
function StrTrim ()
{
    RES=`StrLTrim "$1"`
    RES=`StrRTrim "$RES"`
    echo "$RES"
}

# $1 - func arg
# out - func arg name
function GetFuncArgName()
{
    echo "$1" | awk '{ printf $NF; }'
}

# $1 - func arg
# out - func arg type
function GetFuncArgType()
{
    echo "$1" | awk '{ for (i = 1; i < (NF - 1); i++) printf $i " "; printf $i; }'
}

if [ -z "$6" ]; then
    echo "Usage: gen-yagl-calls.sh [input.in] [api] [target-guard-name] [target-header-include-files] [target-source-include-files] [target-output-basename] [host-guard-name] [host-symbol-prefix] [host-source-include-files] [host-output-basename]"
    exit 1
fi;

API="$2"
TARGET_GUARD_NAME="$3"
TARGET_HEADER_INCLUDE_FILES="$4"
TARGET_SOURCE_INCLUDE_FILES="$5"
TARGET_HEADER_FILE="$6.h"
TARGET_SOURCE_FILE="$6.c"
TARGET_IMPL_FILE="${6}_impl.txt"
HOST_GUARD_NAME="$7"
HOST_SYMBOL_PREFIX="$8"
HOST_SOURCE_INCLUDE_FILES="$9"
HOST_HEADER_FILE="${10}.h"
HOST_SOURCE_FILE="${10}.c"
HOST_IMPL_FILE="${10}_impl.txt"

echo -n "" > $TARGET_HEADER_FILE
echo -n "" > $TARGET_SOURCE_FILE
echo -n "" > $TARGET_IMPL_FILE
echo -n "" > $HOST_HEADER_FILE
echo -n "" > $HOST_SOURCE_FILE
echo -n "" > $HOST_IMPL_FILE

cat >>$TARGET_HEADER_FILE <<EOF
/*
 * Generated by gen-yagl-calls.sh, do not modify!
 */
#ifndef _${TARGET_GUARD_NAME}_H_
#define _${TARGET_GUARD_NAME}_H_

#include "yagl_export.h"
#include "yagl_types.h"
EOF

cat >>$TARGET_SOURCE_FILE <<EOF
/*
 * Generated by gen-yagl-calls.sh, do not modify!
 */
EOF

cat >>$HOST_HEADER_FILE <<EOF
/*
 * Generated by gen-yagl-calls.sh, do not modify!
 */
#ifndef _${HOST_GUARD_NAME}_H_
#define _${HOST_GUARD_NAME}_H_

#include "yagl_types.h"
EOF

cat >>$HOST_SOURCE_FILE <<EOF
/*
 * Generated by gen-yagl-calls.sh, do not modify!
 */
EOF

IFS=$','
for INCLUDE_FILE in $TARGET_HEADER_INCLUDE_FILES; do
    cat >>$TARGET_HEADER_FILE <<EOF
#include <$INCLUDE_FILE>
EOF
done;
for INCLUDE_FILE in $TARGET_SOURCE_INCLUDE_FILES; do
    cat >>$TARGET_SOURCE_FILE <<EOF
#include "$INCLUDE_FILE"
EOF
done;
for INCLUDE_FILE in $HOST_SOURCE_INCLUDE_FILES; do
    cat >>$HOST_SOURCE_FILE <<EOF
#include "$INCLUDE_FILE"
EOF
done;
unset IFS

cat >>$TARGET_SOURCE_FILE <<EOF
#include "yagl_state.h"
#include <assert.h>
EOF

cat >>$HOST_SOURCE_FILE <<EOF
#include "yagl_log.h"
EOF

echo "" >> $TARGET_HEADER_FILE
echo "" >> $TARGET_SOURCE_FILE
echo "" >> $HOST_SOURCE_FILE

cat >>$HOST_HEADER_FILE <<EOF

extern const uint32_t ${HOST_SYMBOL_PREFIX}_num_funcs;

extern yagl_api_func ${HOST_SYMBOL_PREFIX}_funcs[];

#endif
EOF

FUNC_POINTERS=""
FUNC_ID=1
while read -r CUR_LINE || [ -n "$CUR_LINE" ]; do
    CUR_LINE=`StrTrim "$CUR_LINE"`
    FORCE_SYNC=0
    if [ -z "$CUR_LINE" ] ||
       [ "${CUR_LINE:0:1}" == "#" ]; then
        continue;
    fi;
    if [ "${CUR_LINE:0:1}" == "@" ]; then
        CUR_LINE="${CUR_LINE:1}"
        FORCE_SYNC=1
    fi;
    RET_TYPE="$(StrTrim "`echo "$CUR_LINE" 2>/dev/null | cut -f 1 -d,`")"
    if [ "`echo "$RET_TYPE" | grep \*`" != "" ]; then
        RET_TYPE_GETTER="yagl_marshal_get_ptr"
        RET_TYPE_SETTER="yagl_marshal_put_ptr"
    else
        if [ "`echo "$RET_TYPE" | grep \\\^`" != "" ]; then
            RET_TYPE_GETTER="yagl_marshal_get_host_handle"
            RET_TYPE_SETTER="yagl_marshal_put_host_handle"
        else
            RET_TYPE_GETTER="yagl_marshal_get_${RET_TYPE}"
            RET_TYPE_SETTER="yagl_marshal_put_${RET_TYPE}"
        fi;
    fi;
    if [ "`echo "$RET_TYPE" | grep \\\^`" != "" ]; then
        RET_TYPE="`echo "$RET_TYPE" | sed 's,.*^,yagl_host_handle,g'`"
    fi;
    FUNC_NAME="$(StrTrim "`echo "$CUR_LINE" 2>/dev/null | cut -f 2 -d,`")"
    FUNC_ARGS="$(StrTrim "`echo "$CUR_LINE" 2>/dev/null | cut -f 3- -d,`")"
    if [ -z "$RET_TYPE" ] ||
       [ -z "$FUNC_NAME" ]; then
        echo "Bad line in $1: $CUR_LINE"
        exit 1
    fi;
    echo "Processing ${FUNC_NAME}..."
    cat >>$TARGET_HEADER_FILE <<EOF
/*
 * ${FUNC_NAME} wrapper. id = ${FUNC_ID}
 */
EOF
    cat >>$TARGET_SOURCE_FILE <<EOF
/*
 * ${FUNC_NAME} wrapper. id = ${FUNC_ID}
 */
EOF
    cat >>$HOST_SOURCE_FILE <<EOF
/*
 * ${FUNC_NAME} dispatcher. id = ${FUNC_ID}
 */
EOF
    if [ "$RET_TYPE" != "void" ]; then
        echo -n "int yagl_host_${FUNC_NAME}(${RET_TYPE}* retval" >> $TARGET_HEADER_FILE
        echo -n "int yagl_host_${FUNC_NAME}(${RET_TYPE}* retval" >> $TARGET_SOURCE_FILE
        echo -n "bool yagl_host_${FUNC_NAME}(${RET_TYPE}* retval" >> $HOST_IMPL_FILE
    else
        echo -n "int yagl_host_${FUNC_NAME}(" >> $TARGET_HEADER_FILE
        echo -n "int yagl_host_${FUNC_NAME}(" >> $TARGET_SOURCE_FILE
        echo -n "bool yagl_host_${FUNC_NAME}(" >> $HOST_IMPL_FILE
    fi;
    echo "static bool yagl_func_${FUNC_NAME}(struct yagl_thread_state *ts," >> $HOST_SOURCE_FILE
    echo "    uint8_t **out_buff," >> $HOST_SOURCE_FILE
    echo "    uint8_t *in_buff)" >> $HOST_SOURCE_FILE
    if [ "$FUNC_ID" != 1 ]; then
        FUNC_POINTERS="${FUNC_POINTERS},\\n"
    fi;
    FUNC_POINTERS="${FUNC_POINTERS}    &yagl_func_${FUNC_NAME}"
    IFS=$','
    I=0
    for FUNC_ARG in ${FUNC_ARGS}; do
        if [ "$I" != 0 ] || [ "$RET_TYPE" != "void" ]; then
            echo -n ", " >> $TARGET_HEADER_FILE
            echo -n ", " >> $TARGET_SOURCE_FILE
        fi;
        FUNC_ARG="$(StrTrim "$FUNC_ARG")"
        FUNC_ARG_NAME="$(GetFuncArgName "$FUNC_ARG")"
        FUNC_ARG_TYPE="$(GetFuncArgType "$FUNC_ARG")"
        if [ "`echo "$FUNC_ARG_TYPE" | grep \\\^`" != "" ]; then
            FUNC_ARG_TYPE="`echo "$FUNC_ARG_TYPE" | sed 's,.*^,yagl_host_handle,g'`"
        fi;
        echo -n "${FUNC_ARG_TYPE} ${FUNC_ARG_NAME}" >> $TARGET_HEADER_FILE
        echo -n "${FUNC_ARG_TYPE} ${FUNC_ARG_NAME}" >> $TARGET_SOURCE_FILE
        I=$(($I + 1))
    done;
    unset IFS
    if [ "$RET_TYPE" != "void" ]; then
        echo -n "YAGL_IMPLEMENT_API_RET$I($RET_TYPE, $FUNC_NAME" >> $TARGET_IMPL_FILE
    else
        echo -n "YAGL_IMPLEMENT_API_NORET$I($FUNC_NAME" >> $TARGET_IMPL_FILE
    fi
    echo ");" >> $TARGET_HEADER_FILE
    echo "" >> $TARGET_HEADER_FILE
    echo ")" >> $TARGET_SOURCE_FILE
    cat >>$TARGET_SOURCE_FILE <<EOF
{
    uint8_t* base = yagl_batch_get_marshal();
    yagl_marshal_put_uint32(&base, ${API});
    yagl_marshal_put_uint32(&base, ${FUNC_ID});
EOF
    cat >>$HOST_SOURCE_FILE <<EOF
{
EOF
    IFS=$','
    I=0
    for FUNC_ARG in ${FUNC_ARGS}; do
        if [ "$I" != 0 ] || [ "$RET_TYPE" != "void" ]; then
            echo "," >> $HOST_IMPL_FILE
            echo -n "    " >> $HOST_IMPL_FILE
        fi;
        FUNC_ARG="$(StrTrim "$FUNC_ARG")"
        FUNC_ARG_NAME="$(GetFuncArgName "$FUNC_ARG")"
        FUNC_ARG_TYPE="$(GetFuncArgType "$FUNC_ARG")"
        TMP_FUNC_ARG_TYPE="$FUNC_ARG_TYPE"
        if [ "`echo "$FUNC_ARG_TYPE" | grep \\\^`" != "" ]; then
            TMP_FUNC_ARG_TYPE="`echo "$FUNC_ARG_TYPE" | sed 's,.*^,yagl_host_handle,g'`"
        fi;
        if [ "`echo "$FUNC_ARG_TYPE" | grep \*`" != "" ]; then
            echo "    yagl_marshal_put_ptr(&base, ${FUNC_ARG_NAME});" >> $TARGET_SOURCE_FILE
            echo "    target_ulong ${FUNC_ARG_NAME} = yagl_marshal_get_ptr(out_buff);" >> $HOST_SOURCE_FILE
            echo -n "target_ulong /* ${FUNC_ARG_TYPE} */ ${FUNC_ARG_NAME}" >> $HOST_IMPL_FILE
        else
            echo -n "    $TMP_FUNC_ARG_TYPE ${FUNC_ARG_NAME} = " >> $HOST_SOURCE_FILE
            echo -n "$TMP_FUNC_ARG_TYPE ${FUNC_ARG_NAME}" >> $HOST_IMPL_FILE
            if [ "`echo "$FUNC_ARG_TYPE" | grep \\\^`" != "" ]; then
                echo "    yagl_marshal_put_host_handle(&base, ${FUNC_ARG_NAME});" >> $TARGET_SOURCE_FILE
                echo "yagl_marshal_get_host_handle(out_buff);" >> $HOST_SOURCE_FILE
            else
                echo "    yagl_marshal_put_${FUNC_ARG_TYPE}(&base, ${FUNC_ARG_NAME});" >> $TARGET_SOURCE_FILE
                echo "yagl_marshal_get_${FUNC_ARG_TYPE}(out_buff);" >> $HOST_SOURCE_FILE
            fi;
        fi;
        echo -n ", $TMP_FUNC_ARG_TYPE" >> $TARGET_IMPL_FILE
        I=$(($I + 1))
    done;
    if [ "$I" == 0 ] && [ "$RET_TYPE" == "void" ]; then
        echo -n "void" >> $HOST_IMPL_FILE
    fi;
    echo -n "    YAGL_LOG_FUNC_ENTER_SPLIT$I(ts->ps->id, ts->id, $FUNC_NAME" >> $HOST_SOURCE_FILE
    for FUNC_ARG in ${FUNC_ARGS}; do
        FUNC_ARG="$(StrTrim "$FUNC_ARG")"
        FUNC_ARG_NAME="$(GetFuncArgName "$FUNC_ARG")"
        FUNC_ARG_TYPE="$(GetFuncArgType "$FUNC_ARG")"
        if [ "`echo "$FUNC_ARG_TYPE" | grep \\\^`" != "" ]; then
            FUNC_ARG_TYPE="`echo "$FUNC_ARG_TYPE" | sed 's,.*^,yagl_host_handle,g'`"
        fi;
        if [ "`echo "$FUNC_ARG_TYPE" | grep \*`" != "" ]; then
            echo -n ", target_ulong" >> $HOST_SOURCE_FILE
        else
            echo -n ", ${FUNC_ARG_TYPE}" >> $HOST_SOURCE_FILE
        fi;
    done;
    for FUNC_ARG in ${FUNC_ARGS}; do
        FUNC_ARG="$(StrTrim "$FUNC_ARG")"
        FUNC_ARG_NAME="$(GetFuncArgName "$FUNC_ARG")"
        echo -n ", ${FUNC_ARG_NAME}" >> $HOST_SOURCE_FILE
        echo -n ", ${FUNC_ARG_NAME}" >> $TARGET_IMPL_FILE
    done;
    echo ");" >> $HOST_SOURCE_FILE
    if [ "$RET_TYPE" != "void" ]; then
        echo "    $RET_TYPE retval;" >> $HOST_SOURCE_FILE
        echo -n "    bool res = yagl_host_${FUNC_NAME}(&retval" >> $HOST_SOURCE_FILE
    else
        echo -n "    bool res = yagl_host_${FUNC_NAME}(" >> $HOST_SOURCE_FILE
    fi
    I=0
    for FUNC_ARG in ${FUNC_ARGS}; do
        if [ "$I" != 0 ] || [ "$RET_TYPE" != "void" ]; then
            echo -n ", " >> $HOST_SOURCE_FILE
        fi;
        FUNC_ARG="$(StrTrim "$FUNC_ARG")"
        FUNC_ARG_NAME="$(GetFuncArgName "$FUNC_ARG")"
        echo -n "${FUNC_ARG_NAME}" >> $HOST_SOURCE_FILE
        I=$(($I + 1))
    done;
    unset IFS
    echo ");" >> $HOST_SOURCE_FILE
    echo ");" >> $HOST_IMPL_FILE
    cat >>$TARGET_SOURCE_FILE <<EOF
    if (!yagl_batch_update_marshal(base))
    {
        return 0;
    }
EOF
    if [ "$RET_TYPE" != "void" ]; then
        cat >>$TARGET_SOURCE_FILE <<EOF
    if (!yagl_batch_sync())
    {
        return 0;
    }
    base = yagl_batch_get_marshal();
    yagl_marshal_skip(&base); // call result
    *retval = ${RET_TYPE_GETTER}(&base);
    return 1;
EOF
        cat >>$HOST_SOURCE_FILE <<EOF
    if (!res) {
        YAGL_LOG_FUNC_EXIT(NULL);
        return false;
    }
    YAGL_LOG_FUNC_EXIT_SPLIT(${RET_TYPE}, retval);
    ${RET_TYPE_SETTER}(&in_buff, retval);
    return true;
EOF
    else
        if [ "$FORCE_SYNC" == 1 ]; then
            cat >>$TARGET_SOURCE_FILE <<EOF
    return yagl_batch_sync();
EOF
        else
            cat >>$TARGET_SOURCE_FILE <<EOF
    return 1;
EOF
        fi;
        cat >>$HOST_SOURCE_FILE <<EOF
    YAGL_LOG_FUNC_EXIT(NULL);
    return res;
EOF
    fi
    echo "}" >> $TARGET_SOURCE_FILE
    echo "" >> $TARGET_SOURCE_FILE
    echo ")" >> $TARGET_IMPL_FILE
    echo "}" >> $HOST_SOURCE_FILE
    echo "" >> $HOST_SOURCE_FILE
    FUNC_ID=$(($FUNC_ID + 1))
done < $1

RETVAL="$?"
if [ "$RETVAL" != 0 ]; then
    exit 1
fi;

cat >>$TARGET_HEADER_FILE <<EOF
#endif
EOF

cat >>$HOST_SOURCE_FILE <<EOF
const uint32_t ${HOST_SYMBOL_PREFIX}_num_funcs = $((${FUNC_ID} - 1));

yagl_api_func ${HOST_SYMBOL_PREFIX}_funcs[] = {
EOF
echo -e "$FUNC_POINTERS" >> $HOST_SOURCE_FILE
cat >>$HOST_SOURCE_FILE <<EOF
};
EOF
