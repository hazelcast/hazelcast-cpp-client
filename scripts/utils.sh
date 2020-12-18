kill_background_jobs() {
  for pid in $(jobs -p); do
    kill -9 $pid
  done
}
