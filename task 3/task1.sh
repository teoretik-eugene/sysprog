#!/usr/bin/env bash

echo "Запуск родительского процесса PID=$$"

# Запуск дочернего процесса в фоновом режиме
./task1_child &
CHILD_PID=$!
echo "Запущен дочерний процесс с PID=$CHILD_PID"

FLAG=$(ps aux | grep task1_child | awk '{print $11, $12}' | tac | tail -1)

if [ "$FLAG" == "bash ./task1_child" ]; then

    sleep 5

    echo "Родительский процесс PID=$$ отправил SIGTERM дочернему PID=$CHILD_PID."
    kill -SIGKILL $CHILD_PID

    wait $CHILD_PID
    echo "Дочерний процесс PID=$CHILD_PID завершен."

fi