#!/bin/sh

case "$1" in
  start)
    echo "Starting simpleserver"
    start-stop-daemon -S -n simpleserver -a/usr/bin/simpleserver
    ;;
  stop)
    echo "Stopping simpleserver"
    ;;
  *)
    echo "Usage: $0 (start|stop)"
  exit 1
esac

exit 0
