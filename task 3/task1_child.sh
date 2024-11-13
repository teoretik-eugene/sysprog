#!/usr/bin/env bash

echo "Дочерний процесс PID: $$"

echo "$(ps aux | grep task1_child | tac | tail -1)"

echo "--Дочерний процесс бесконечно ожидает--"
sleep infinity