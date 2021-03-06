/**************************************************************/
/*
 * Copyright (c) 2014, Ohad Rodeh, IBM Research
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of IBM Research.
 * 
 */
/**************************************************************/
#ifndef OC_CRT_INT_H
#define OC_CRT_INT_H

#include "pl_dstru.h"
#include "oc_utl.h"
#include "oc_crt_s.h"

void oc_crt_init_full(Oc_crt_config * config_p);
void oc_crt_default_config(Oc_crt_config *config_p);
void oc_crt_init(void);
Oc_crt_config *oc_crt_get_config(void);

// assert that the caller is on a co-routine stack
void oc_crt_assert(void);

// Return the pthread-id used by the co-routines
int oc_crt_get_thread (void);

/* The CRT uses virtual time in Harness mode, therefore,
 * all uses of time should go through this function.
 *
 * Return time in milliseconds since initialization
 */
uint64 oc_crt_get_time_millis(void);

/* Fork a new task to run function [run_p]. 
 *
 * can -only- be called from the pthread running co-routines.
 */
void oc_crt_create_task(const char * name_p,
                        void (*run_p) (void *),
                        void * arg_p);

/* Wait until a task-structure is available, and then start the task.
 * 
 * can -only- be called by a co-routine.
 */
void oc_crt_create_task_b(const char * name_p,
                          void (*run_p) (void *),
                          void * arg_p);

Oc_crt_task *oc_crt_get_current_task(void);

// kills all tasks except current one
void oc_crt_kill_all();

void oc_crt_throw(void);

#define OC_CRT_TRY \
    { \
        Oc_crt_task * current = oc_crt_get_current_task(); \
        if(current == NULL) { \
              ERR(("Cant do try not inside coroutine %p\n", current)); \
        } \
        if(current->exception_is_set == TRUE) { \
              ERR(("Exception has already been set\n")); \
        } \
        current->exception_caught = 0; \
        current->exception_is_set = TRUE; \
        if(setjmp(current->exception) == 0) {
             /* code inside the TRY */
#define OC_CRT_CATCH \
        } \
        else {\
             oc_utl_assert(current->exception_is_set == TRUE); \
             current->exception_caught = 1; \
        } \
    } \
    oc_crt_get_current_task()->exception_is_set = FALSE; \
    if (oc_crt_get_current_task()->exception_caught)
    /* code inside catch */

#define OC_CRT_THROW oc_crt_throw()

#define OC_CRT_THROW_DBG oc_utl_assert(0)

void oc_crt_sleep(uint32 milisec);

void oc_crt_yield_task(void);

void oc_crt_init_wait_q(Oc_crt_wait_q * wq);
void oc_crt_wait_on_q(Oc_crt_wait_q * wq);
void oc_crt_wake_up_q(Oc_crt_wait_q * wq);
void oc_crt_wake_up_all(Oc_crt_wait_q * wq);
int  oc_crt_wait_q_len(Oc_crt_wait_q * wq);
int  oc_crt_is_wait_q_empty(Oc_crt_wait_q * wq);

// RW locks
void oc_crt_init_rw_lock(Oc_crt_rw_lock * lock);
void oc_crt_lock_read(Oc_crt_rw_lock * lock);
void oc_crt_lock_write(Oc_crt_rw_lock * lock);

// downgrade from write-mode to read-mode
void oc_crt_lock_downgrade(Oc_crt_rw_lock *lock_p);
void oc_crt_unlock(Oc_crt_rw_lock * lock);

// return TRUE if lock is taken for write, FALSE otherwise
bool oc_crt_rw_is_locked_write(Oc_crt_rw_lock * lock);
bool oc_crt_rw_is_locked_read(Oc_crt_rw_lock * lock_p);
bool oc_crt_lock_check(Oc_crt_rw_lock * lock_p);

// Semaphores
void oc_crt_sema_init(Oc_crt_sema * sema_p, int value);
void oc_crt_sema_post(Oc_crt_sema * sema_p);
void oc_crt_sema_wait(Oc_crt_sema * sema_p);
int  oc_crt_sema_get_val(Oc_crt_sema * sema_p);

/* Binary semaphores (b-semaphore).
 * 
 * A binary semaphore can count only between 0 and 1.
 *
 * Posting to a b-semaphore increases its value to 1. Subsequent
 * post operations leave the value at 1. A wait operation reduces
 * the value to zero.
 */
void oc_crt_bn_sema_init(Oc_crt_bn_sema * sema_p, int value);
void oc_crt_bn_sema_post(Oc_crt_bn_sema * sema_p);
void oc_crt_bn_sema_wait(Oc_crt_bn_sema * sema_p);
int  oc_crt_bn_sema_get_val(Oc_crt_bn_sema * sema_p);

// Atomic counters
void oc_crt_atom_cnt_init(Oc_crt_atom_cnt * acnt_p, int init_num_i);
void oc_crt_atom_cnt_inc(Oc_crt_atom_cnt * acnt_p, int num_i);
void oc_crt_atom_cnt_dec(Oc_crt_atom_cnt * acnt_p, int num_i);
int  oc_crt_atom_cnt_get_val(Oc_crt_atom_cnt * acnt_p);

// Thread-safe semaphores
void oc_crt_ts_sema_init(Oc_crt_ts_sema * sema_p, int value);
void oc_crt_ts_sema_post(Oc_crt_ts_sema * sema_p);
void oc_crt_ts_sema_wait(Oc_crt_ts_sema * sema_p);
int  oc_crt_ts_sema_pending(Oc_crt_ts_sema * sema_p);
int  oc_crt_ts_sema_get_val(Oc_crt_ts_sema * sema_p);

#endif

