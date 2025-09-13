# Links

- https://scholar.google.com/scholar?hl=de&as_sdt=0%2C5&q=fastflow+vs+equeue&btnG=
- https://doc.dpdk.org/guides/prog_guide/ring_lib.html
- [FastFlow](https://link.springer.com/chapter/10.1007/978-3-642-23397-5_17)
- [EQueue](https://doi.org/10.1109/ACCESS.2020.2997071)
    - associated github repository was taken down after I wrote an email to the authors asking some things
      about paper and repo
    - since repo was licensed under GPLv3, I reuploaded it: https://github.com/Computerdores/equeue
- [B-Queue](https://doi.org/10.1007/s10766-012-0213-x)
    - Sourceforge Repo is dead, [fork on github](https://github.com/olibre/B-Queue) exists
      - https://psy-lob-saw.blogspot.com/2013/11/spsc-iv-look-at-bqueue.html
    - pseudo code from papers seems to be incorrect as I had to fix two bugs in it to stop deadlocks
- [CFCLF Queue](https://doi.org/10.1109/ICCSN.2017.8230170)
- [MCRingBuffer](https://doi.org/10.1145/1882486.1882508)
  - number of elements enqueued has to be divisible by the batch size,
    otherwise deadlock occurs when consumer waits for producer to finish its batch

# Queue Candidates


# Infos

- "Style: Ten Lessons In Clarity And Grace" by Joseph Williams

## Other papers
- "latency of B-queue operations can be as low as 20 cycles" - https://arxiv.org/html/2502.05293v2#:~:text=XQueue%20uses%20B,as%20low%20as%2020%20cycles
