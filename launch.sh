#!/bin/sh
set -e

CURDIR=`pwd`

APP_NAME="KotlTester"

cd /home/root

if [ -e "./${APP_NAME}" ]; then

    chmod +x ./${APP_NAME}

else

    echo "not found the app file : ${APP_NAME}"
    exit 1

fi


if [ ! -e "./logs" ]; then
    mkdir -p ./logs
fi

# exec ps

pid=`ps | grep "${APP_NAME}" | grep -v "grep" | awk '{print $1;exit}'`

if [ ! -z ${pid} ]; then
    echo "found app: ${APP_NAME} = ${pid}"

	kill -10 ${pid}

    sleep 2
else
    echo "no found running app:${APP_NAME} = ${pid}"
fi

MONITOR_LOG_FILE="./logs/app_running_monitor.log"
ls -l ./logs/

if [ -e "./logs/${APP_NAME}_3.log" ]; then
    
    if [ -e "./logs/${APP_NAME}_1.log" ]; then
        rm -f ./logs/${APP_NAME}_1.log
    fi

    if [ -e "./logs/${APP_NAME}_2.log" ]; then
        mv ./logs/${APP_NAME}_2.log ./logs/${APP_NAME}_1.log
    fi

    mv ./logs/${APP_NAME}_3.log ./logs/${APP_NAME}_2.log
fi

# exec ps


current_date=`date +"%F %T %z"`
echo "${current_date} start app:${APP_NAME}" >> ${MONITOR_LOG_FILE}

echo "${current_date} start app : ${APP_NAME}"

nohup ./${APP_NAME} > ./logs/${APP_NAME}_3.log 2>&1  &

APP_RUNNING_STATUS="reload"
echo ${APP_RUNNING_STATUS} > ./apprunningstatus

while [ "${APP_RUNNING_STATUS}" = "reload" ]; do

	# APP_RUNNING_STATUS=""
	# if [ -e "./apprunningstatus" ]; then
	# 	echo "" > ./apprunningstatus
	# fi
    sleep 5

    pid=`ps | grep "${APP_NAME}" | grep -v "grep" | awk '{print $1;exit}'`

    if [ ! -z ${pid} ]; then
        

        # kill -10 ${pid}
        current_date=`date +"%F %T %z"`
        echo "${current_date} app: ${APP_NAME} is running : ${pid}" >> ${MONITOR_LOG_FILE}
        echo "${current_date} app: ${APP_NAME} is running : ${pid}"

        sleep 10
    else
        echo "app:${APP_NAME} stopped."

        if [ -e "./logs/${APP_NAME}_3.log" ]; then
    
            if [ -e "./logs/${APP_NAME}_1.log" ]; then
                rm -f ./logs/${APP_NAME}_1.log
            fi

            if [ -e "./logs/${APP_NAME}_2.log" ]; then
                mv ./logs/${APP_NAME}_2.log ./logs/${APP_NAME}_1.log
            fi

            mv ./logs/${APP_NAME}_3.log ./logs/${APP_NAME}_2.log
        fi

        current_date=`date +"%F %T %z"`
        echo "${current_date} restart app:${APP_NAME}" >> ${MONITOR_LOG_FILE}
        echo "${current_date} restart app : ${APP_NAME}"

        nohup ./${APP_NAME} > ./logs/${APP_NAME}_3.log 2>&1  &

        sleep 5
    fi

    if [ -e "./apprunningstatus" ]; then
		APP_RUNNING_STATUS=`cat ./apprunningstatus`
	fi

	echo "app next status: ${APP_RUNNING_STATUS}"

done

echo "app will exited"
