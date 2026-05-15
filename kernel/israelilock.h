struct israelilock {
  uint safe_lock;       // lock for the israelilock for safelty
  uint locked;          // Is the lock held?
  uint destroyed;       // 1 = destroyed, 0 = active
  uint favoritism; 
  int queue[16];  
  int queue_count;      // Number of processes currently waiting
  int last_holder_gid;  // The gid of the process that last held the lock
  
  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
};

