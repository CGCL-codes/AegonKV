sudo -E /home/hfeng/code/rocksdb/cmake-build-debug-node27/db_bench -benchmarks=fillrandom -perf_level=3 -use_direct_io_for_flush_and_compaction=true \
  -use_direct_reads=true -cache_size=268435456 -key_size=48 \
  -value_size=256 -num=50000000 -db=./db -threads=16 \
  -write_buffer_size=67108864 \
  -max_write_buffer_number=2 \
  -target_file_size_base=8388608 \
  -max_bytes_for_level_base=16777216 \
  -max_bytes_for_level_multiplier=3 \
  -disable_wal=true \
  -compression_type=none \
  -max_background_compactions=9 \
  -max_background_flushes=3 \
  -use_titan=true \
  -titan_max_background_gc=6 \
  -blob_file_discardable_ratio=0.5 \
  -open_files=20000 \
  -blob_db_file_size=33554432 \
  -titan_min_blob_size=0 \
  -max_gc_batch_size=67108864 \
  -min_gc_batch_size=33554432 \
  -gc_time_list_path=./GC_TIME \
  -titan_level_merge=false \
  -titan_range_merge=false \
  -max_sorted_runs=3 \
  -use_bitmap=true \
  -gc_offload=true \
  -binary_file_path=/home/hfeng/code/titan/cmake-build-debug-node27/hardware/kernel/hw/gc.xclbin

nohup \
  sudo -E \
  /home/hfeng/code/titan/cmake-build-debug-node27/titandb_bench \
  -benchmarks="fillrandom,mixgraph" \
  -use_direct_io_for_flush_and_compaction=true -use_direct_reads=true -cache_size=268435456 \
  -keyrange_dist_a=14.18 -keyrange_dist_b=-2.917 -keyrange_dist_c=0.0164 -keyrange_dist_d=-0.08082 -keyrange_num=30 \
  -value_k=0.923 -value_sigma=226.409 \
  -iter_k=2.517 -iter_sigma=14.236 \
  -mix_get_ratio=0.55 -mix_put_ratio=0.44 -mix_seek_ratio=0.01 \
  -sine_mix_rate_interval_milliseconds=5000 -sine_a=1000 -sine_b=0.000073 -sine_d=80000 \
  -perf_level=2 -reads=420000000 -num=50000000 -key_size=48 -value_size=256 -threads=16 \
  -write_buffer_size=67108864 \
  -max_write_buffer_number=2 \
  -target_file_size_base=8388608 \
  -max_bytes_for_level_base=16777216 \
  -max_bytes_for_level_multiplier=3 \
  -statistics=true \
  -db=./db \
  -disable_wal=true \
  -compression_type=none \
  -max_background_compactions=9 \
  -max_background_flushes=3 \
  -use_titan=true \
  -titan_max_background_gc=6 \
  -blob_file_discardable_ratio=0.5 \
  -open_files=20000 \
  -blob_db_file_size=33554432 \
  -titan_min_blob_size=0 \
  -max_gc_batch_size=67108864 \
  -min_gc_batch_size=33554432 \
  -gc_time_list_path=./GC_TIME \
  -titan_level_merge=false \
  -titan_range_merge=false \
  -max_sorted_runs=3 \
  -use_bitmap=true \
  -gc_offload=true \
  -binary_file_path=/home/hfeng/code/titan/cmake-build-debug-node27/hardware/kernel/hw/gc.xclbin \
  >mixgraph 2>&1 &

nohup \
  sudo -E \
  python3 /home/hfeng/code/YCSB/titandb/cpu_util.py 1007533 \
  >/dev/null 2>&1 &

nohup \
  sudo -E \
  python3 /home/hfeng/code/YCSB/titandb/size_util.py 1007533 \
  >/dev/null 2>&1 &

nohup \
  python3 /home/hfeng/code/YCSB/titandb/cpu_util.py 1007533 \
  >/dev/null 2>&1 &

nohup \
  python3 /home/hfeng/code/YCSB/titandb/size_util.py 1007533 \
  >/dev/null 2>&1 &
