# Links
- https://scholar.google.com/scholar?hl=de&as_sdt=0%2C5&q=fastflow+vs+equeue&btnG=
- https://link.springer.com/article/10.1007/s10766-012-0213-x
- https://doc.dpdk.org/guides/prog_guide/ring_lib.html
- https://link.springer.com/chapter/10.1007/978-3-642-23397-5_17

# Queue Candidates
- CFCLF

# Infos
- "Style: Ten Lessons In Clarity And Grace" by Joseph Williams


# Time measurement
- `test_tsc_freq` seems to show that the tsc freq on laptop and office pc are **not** constant
- `test_mc_sync` seems to show that the cross core synchronisation is ~~out of sync by less than 1-2ms about 50% of the time~~
  - ~~the distribution is bimodal, with one peak at \~(1-2ms, 45%) and one smaller peak that varies more strongly between `CLOCK_MONOTONIC_RAW` and `RDTSC`~~
  - ~~imo most likely explanation is that cpu scheduling causes the smaller peak and that the 1-2ms are the actual value~~
    - ~~needs to be verified -> kernel mod to disable scheduling for some cores (cpusets would be a good tool for this)~~
  - cpu affinity was being set incorrectly
  - actual result: in almost all cases the difference is below 300-400ns for TSC even below 200ns (values are spotty for TSC though)
