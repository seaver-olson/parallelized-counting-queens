Author: Seaver Olson
Project 2B
Comp 364

Part 1: Using perf stat
  1. As you add workers, does the total instruction count stay about the same? If it grows, where are 
the extra instructions coming from? (Hint: a thread or worker that is waiting isn’t always asleep.) 
  I unfortunately am using WSL2 which does not report hardware performance counters.  Running perf stat with the instructions:u event returned <not supported>.  I would say when I add workers my total instruction count would most likely grow but too what extent I do not know.  Adding workings means when they are not running a workload they are stalling depending on the workload distribution this can lead some workers to actively poll for work a lot.

  2. How does “CPUs utilized” compare to the number of workers you asked for? Compare static and dynamic. 
    I found during my benchmarks that the more workers I added, the greater the difference in CPU utilization between static and dynamic openMP.  At a max of 24 workers, the CPU utilization on the static schedule was 74.2% (17.813 CPUs utilized) while the dynamic schedule reached 93.2% (22.378 CPUs utilized).  This makes a lot of sense because I am benchmarking N-Queens which when distributed to workers can give some workers very intense workloads while some workers have very little work.  With the dynamic scheduling, when the workers finish their task they get another task until their are none remaining.  This allows the CPUs to not stall waiting for other CPUs to be done.

  3. For MPI: do all workers execute about the same number of instructions? What does that tell you about your task breakdown? Make a bar chart of instructions per worker. 

  4. N-Queens is a search full of branches. What’s your branch-miss rate?

  5. What’s your cache-miss rate? Use your perf data and the size of your board to explain whether the 
cache matters for this problem. 

Part 2: Flame Graphs with perf record 
  1. What fraction of samples are from your own code, and what fraction are from the OpenMP runtime (functions starting with gomp_ or GOMP_) or the MPI library? Report the proportional runtime. 
  I profiled the serial implementation and my four-thread OpenMP dynamic implementation using perf record with the cpu-clock event.  I originally made the OpenMP dynamic implementation with 24 workers, but I reduced it because the flame graph was very difficult to read.  For the serial implementation, 99.99% of sampled CPU time was in the recursive solve() function, with only .01% attributed to the linux kernel.  For the OpenMP dynamic implementation, 99.97% of samples were attributed to solve() while .03% was spent in the OpenMP runtime library.  The linux kernel for this test only accounted for .01% of time.  These results tell me that the recursive search dominates the CPU in both implementations.  OpenMP runtime functions account for a small amount of the sampled CPU time even with the additional scheduling needed.  This points to parallelization being very efficent for this program and does not cause much overhead.
  
  2. Your search is recursive. Describe what the recursion looks like in the flame graph and why. 
    In my graph, solve is stacked on top of itself, forming a vertical column with the solve() width growing smaller every call.  This happenes because each recursive call happens when a queen is placed meaning the num of columns is reduced.  This then calls the same function but analyizes one less column each time.  
  
  3. Observer effect: time each recorded program with and without perf record. How much does 
recording slow it down? Do the same for perf stat. Which one would you trust for timing, and why? 

Part 3: using /proc

