#!/bin/bash
# file build-bismon-temporary-module.sh
MODULEID=$1
logger --id=$$ -s -t build-bismon-temporary-module -p user.debug starting moduleid $MODULEID in $(pwd) at $(date +%c)
make -f $(dirname $0)/Makefile -e "VPATH=$(pwd):$(dirname $0)" modubin/tmpmobm$MODULEID.so
makestatus=$?
if [ $makestatus -ne 0 ]; then
    logger --id=$$ -s -t build-bismon-temporary-module -p user.debug make for $MODULEID failed with $makestatus
    exit $makestatus
else
    logger --id=$$ -s -t build-bismon-temporary-module -p user.debug make for $MODULEID completed
fi

