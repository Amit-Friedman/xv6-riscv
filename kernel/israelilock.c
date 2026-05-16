#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "israelilock.h"
#include "proc.h"
#include "defs.h"

// The external array initialized in proc.c
extern struct israelilock israelilocks[15];
extern struct proc proc[NPROC];

// ---------------------------------------------------------
// Helper Functions for Atomic Synchronization
// ---------------------------------------------------------

// Acquire the internal safeguard lock using atomic built-ins
void safe_lock_acquire(struct israelilock *ilock) {
  // loop until the lock is acquired
  while(__sync_lock_test_and_set(&ilock->safe_lock, 1) != 0) {
    // wait
  }
  __sync_synchronize();
}

// Release the internal safeguard lock
void safe_lock_release(struct israelilock *ilock) {
  __sync_synchronize();
  __sync_lock_release(&ilock->safe_lock);
}

// Helper to find a process's GID given its PID
int get_gid_by_pid(int pid) {
  struct proc *p;
  for(p = proc; p < &proc[NPROC]; p++) {
    if(p->pid == pid) {
      return p->gid;
    }
  }
  return -1; 
}

// ---------------------------------------------------------
// System Call Implementations
// ---------------------------------------------------------

int israeli_create(int favoritism) {
  if(favoritism < 0 || favoritism > 100) return -1;
  for (int i = 0; i < 15; i++) {
    safe_lock_acquire(&israelilocks[i]);
    
    // Find an inactive lock
    if (israelilocks[i].destroyed) {
      israelilocks[i].destroyed = 0;
      israelilocks[i].favoritism = favoritism;
      
      safe_lock_release(&israelilocks[i]);
      return i; // Return the lock_id
    }
    
    safe_lock_release(&israelilocks[i]);
  }
  return -1; // No available locks
}

int israeli_acquire(int lock_id) {
  if (lock_id < 0 || lock_id >= 15) return -1;
  struct israelilock *ilock = &israelilocks[lock_id];
  struct proc *p = myproc();

  safe_lock_acquire(ilock);
  if (ilock->destroyed) {
    safe_lock_release(ilock);
    return -1;
  }
  
  // Add current process to the end of the queue
  if(ilock->locked == 0 && ilock->queue_count == 0){
    ilock->locked = 1;
    safe_lock_release(ilock);
    return 0;
  }
  else if (ilock->queue_count < 16) {
    ilock->queue[ilock->queue_count] = p->pid;
    ilock->queue_count++;
  } else {
    // Failsafe: queue is full
    safe_lock_release(ilock);
    return -1; 
  }
  safe_lock_release(ilock);

  // Wait loop: Yield until it is this process's turn
  while (1) {
    safe_lock_acquire(ilock);
    
    // Check if the lock is free AND this process is at the front of the line
    if (ilock->locked == 0 && ilock->queue[0] == p->pid) {
      ilock->locked = 1;
      
      // Remove self from the queue and shift the rest forward
      for (int i = 0; i < ilock->queue_count - 1; i++) {
        ilock->queue[i] = ilock->queue[i + 1];
      }
      ilock->queue_count--;
      ilock->queue[ilock->queue_count] = -1; // Clean up the trailing slot
      
      safe_lock_release(ilock);
      return 0; // Lock acquired successfully
    }
    
    safe_lock_release(ilock);
    yield(); // Yield the CPU instead of busy waiting
  }
}

int israeli_release(int lock_id) {
  if (lock_id < 0 || lock_id >= 15) return -1;
  struct israelilock *ilock = &israelilocks[lock_id];
  struct proc *p = myproc();

  safe_lock_acquire(ilock);
  
  ilock->locked = 0;
  ilock->last_holder_gid = p->gid; // Record the gid of the releaser
  
  if (ilock->queue_count > 0) {
    int favored_idx = -1;
    
    // Find the *earliest* process in the queue with the same gid
    for (int i = 0; i < ilock->queue_count; i++) {
      if (get_gid_by_pid(ilock->queue[i]) == p->gid) {
        favored_idx = i;
        break; 
      }
    }
    
    // If a matching process exists, check if favoritism takes effect
    if (favored_idx != -1) {
      // Use the PRNG from Task 0
      if ((kernel_lcg_rand() % 100) < ilock->favoritism) {
        
        // Apply favoritism: extract this process and move it to the front
        int favored_pid = ilock->queue[favored_idx];
        for (int i = favored_idx; i > 0; i--) {
          ilock->queue[i] = ilock->queue[i - 1];
        }
        ilock->queue[0] = favored_pid;
      }
    }
  }
  
  safe_lock_release(ilock);
  return 0;
}

int israeli_destroy(int lock_id) {
  if (lock_id < 0 || lock_id >= 15) return -1;
  struct israelilock *ilock = &israelilocks[lock_id];

  israeli_acquire(lock_id);
  
  safe_lock_acquire(ilock);
  
  ilock->destroyed = 1;
  ilock->locked = 0;
  ilock->queue_count = 0;
  
  // Clear out the queue array
  for (int j = 0; j < 16; j++) {
    ilock->queue[j] = -1;
  }
  
  safe_lock_release(ilock);
  
  return 0;
}