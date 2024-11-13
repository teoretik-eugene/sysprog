#!/usr/bin/env bash

# обработчик сигнала
handle_signal() {
    if [ "$1" != 17 ]; then

        NAME=$(kill -l "$1")
        echo "Процесс PID=$$ получил сигнал: SIG$NAME"

        exit 1
    fi
}

for SIGNAL in {1..64}; do
    trap 'handle_signal '"$SIGNAL" $SIGNAL
done

echo "Скрипт запущен. Отправьте сигнал для завершения."
while true; do
    echo ".."
    sleep 1
done

