/* Gearman server and library
 * Copyright (C) 2008 Brian Aker, Eric Day
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in the parent directory for full text.
 */

/**
 * @file
 * @brief Gearman Declarations
 */

#ifndef __GEARMAN_STATE_H__
#define __GEARMAN_STATE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup gearman_state_state
 */
struct gearman_state_st
{
  struct {
    bool allocated:1;
    bool dont_track_packets:1;
    bool non_blocking:1;
    bool stored_non_blocking:1;
  } options;
  gearman_verbose_t verbose;
  uint32_t con_count;
  uint32_t packet_count;
  uint32_t pfds_size;
  uint32_t sending;
  int last_errno;
  int timeout; // Used by poll()
  gearman_connection_st *con_list;
  gearman_packet_st *packet_list;
  struct pollfd *pfds;
  gearman_log_fn *log_fn;
  const void *log_context;
  gearman_event_watch_fn *event_watch_fn;
  const void *event_watch_context;
  gearman_malloc_fn *workload_malloc_fn;
  const void *workload_malloc_context;
  gearman_free_fn *workload_free_fn;
  const void *workload_free_context;
  char last_error[GEARMAN_MAX_ERROR_SIZE];
};


/**
 * @addtogroup gearman Gearman Declarations
 *
 * This is a low level interface for gearman library instances. This is used
 * internally by both client and worker interfaces, so you probably want to
 * look there first. This is usually used to write lower level clients, workers,
 * proxies, or your own server.
 *
 * There is no locking within a single gearman_state_st structure, so for threaded
 * applications you must either ensure isolation in the application or use
 * multiple gearman_state_st structures (for example, one for each thread).
 *
 * @{
 */


/**
 * Initialize a gearman structure. Always check the return value even if passing
 * in a pre-allocated structure. Some other initialization may have failed. It
 * is not required to memset() a structure before providing it.
 *
 * @param[in] gearman Caller allocated structure, or NULL to allocate one.
 * @return On success, a pointer to the (possibly allocated) structure. On
 *  failure this will be NULL.
 */
GEARMAN_API
gearman_state_st *gearman_state_create(gearman_state_st *gearman, gearman_options_t *options);

/**
 * Clone a gearman structure.
 *
 * @param[in] gearman Caller allocated structure, or NULL to allocate one.
 * @param[in] from Structure to use as a source to clone from.
 * @return Same return as gearman_create().
 */
GEARMAN_API
gearman_state_st *gearman_state_clone(gearman_state_st *gearman, const gearman_state_st *from);

/**
 * Free a gearman structure.
 *
 * @param[in] gearman Structure previously initialized with gearman_create() or
 *  gearman_clone().
 */
GEARMAN_API
void gearman_state_free(gearman_state_st *gearman);

/**
 * Set the error string.
 *
 * @param[in] gearman Structure previously initialized with gearman_create() or
 *  gearman_clone().
 * @param[in] function Name of function the error happened in.
 * @param[in] format Format and variable argument list of message.
 */
GEARMAN_API
void gearman_state_set_error(gearman_state_st *gearman, const char *function,
                       const char *format, ...);

/**
 * Return an error string for last error encountered.
 *
 * @param[in] gearman Structure previously initialized with gearman_create() or
 *  gearman_clone().
 * @return Pointer to a buffer in the structure that holds an error string.
 */
static inline const char *gearman_state_error(const gearman_state_st *gearman)
{
  if (gearman->last_error[0] == 0)
      return NULL;
  return (const char *)(gearman->last_error);
}

/**
 * Value of errno in the case of a GEARMAN_ERRNO return value.
 *
 * @param[in] gearman Structure previously initialized with gearman_create() or
 *  gearman_clone().
 * @return An errno value as defined in your system errno.h file.
 */
static inline int gearman_state_errno(const gearman_state_st *gearman)
{
  return gearman->last_errno;
}

/**
 * Add options for a gearman structure.
 *
 * @param[in] gearman Structure previously initialized with gearman_create() or
 *  gearman_clone().
 * @param[in] options Available options for gearman structures.
 */
GEARMAN_API
gearman_return_t gearman_set_option(gearman_state_st *gearman, gearman_options_t option, bool value);

/**
  @todo remove gearman_set_option() and gearman_remove_options()
 */
static inline void gearman_add_options(gearman_state_st *gearman, gearman_options_t options)
{
  (void)gearman_set_option(gearman, options, true);
}

static inline void gearman_remove_options(gearman_state_st *gearman, gearman_options_t options)
{
  (void)gearman_set_option(gearman, options, true);
}

static inline bool gearman_state_is_non_blocking(gearman_state_st *gearman)
{
  return gearman->options.non_blocking;
}

static inline bool gearman_state_is_stored_non_blocking(gearman_state_st *gearman)
{
  return gearman->options.stored_non_blocking;
}

/**
  @todo fix internals to not require state changes like  this.
 */
static inline void gearman_state_push_non_blocking(gearman_state_st *gearman)
{
  gearman->options.stored_non_blocking= gearman->options.non_blocking;
  gearman->options.non_blocking= true;
}

static inline void gearman_state_pop_non_blocking(gearman_state_st *gearman)
{
  gearman->options.non_blocking= gearman->options.stored_non_blocking;
}

/**
 * Get current socket I/O activity timeout value.
 *
 * @param[in] gearman Structure previously initialized with gearman_create() or
 *  gearman_clone().
 * @return Timeout in milliseconds to wait for I/O activity. A negative value
 *  means an infinite timeout.
 */
GEARMAN_API
int gearman_timeout(gearman_state_st *gearman);

/**
 * Set socket I/O activity timeout for connections in a Gearman structure.
 *
 * @param[in] gearman Structure previously initialized with gearman_create() or
 *  gearman_clone().
 * @param[in] timeout Milliseconds to wait for I/O activity. A negative value
 *  means an infinite timeout.
 */
GEARMAN_API
void gearman_set_timeout(gearman_state_st *gearman, int timeout);

/**
 * Set logging function for a gearman structure.
 *
 * @param[in] gearman Structure previously initialized with gearman_create() or
 *  gearman_clone().
 * @param[in] function Function to call when there is a logging message.
 * @param[in] context Argument to pass into the callback function.
 * @param[in] verbose Verbosity level threshold. Only call function when the
 *  logging message is equal to or less verbose that this.
 */
GEARMAN_API
void gearman_set_log_fn(gearman_state_st *gearman, gearman_log_fn *function,
                        const void *context, gearman_verbose_t verbose);

/**
 * Set custom I/O event callback function for a gearman structure.
 *
 * @param[in] gearman Structure previously initialized with gearman_create() or
 *  gearman_clone().
 * @param[in] function Function to call when there is an I/O event.
 * @param[in] context Argument to pass into the callback function.
 */
GEARMAN_API
void gearman_set_event_watch_fn(gearman_state_st *gearman,
                                gearman_event_watch_fn *function,
                                const void *context);

/**
 * Set custom memory allocation function for workloads. Normally gearman uses
 * the standard system malloc to allocate memory used with workloads. The
 * provided function will be used instead.
 *
 * @param[in] gearman Structure previously initialized with gearman_create() or
 *  gearman_clone().
 * @param[in] function Memory allocation function to use instead of malloc().
 * @param[in] context Argument to pass into the callback function.
 */
GEARMAN_API
void gearman_set_workload_malloc_fn(gearman_state_st *gearman,
                                    gearman_malloc_fn *function,
                                    const void *context);

/**
 * Set custom memory free function for workloads. Normally gearman uses the
 * standard system free to free memory used with workloads. The provided
 * function will be used instead.
 *
 * @param[in] gearman Structure previously initialized with gearman_create() or
 *  gearman_clone().
 * @param[in] function Memory free function to use instead of free().
 * @param[in] context Argument to pass into the callback function.
 */
GEARMAN_API
void gearman_set_workload_free_fn(gearman_state_st *gearman,
                                  gearman_free_fn *function,
                                  const void *context);

/**
 * Free all connections for a gearman structure.
 *
 * @param[in] gearman Structure previously initialized with gearman_create() or
 *  gearman_clone().
 */
GEARMAN_API
void gearman_free_all_cons(gearman_state_st *gearman);

/**
 * Flush the send buffer for all connections.
 *
 * @param[in] gearman Structure previously initialized with gearman_create() or
 *  gearman_clone().
 * @return Standard gearman return value.
 */
GEARMAN_API
gearman_return_t gearman_flush_all(gearman_state_st *gearman);

/**
 * Wait for I/O on connections.
 *
 * @param[in] gearman Structure previously initialized with gearman_create() or
 *  gearman_clone().
 * @return Standard gearman return value.
 */
GEARMAN_API
gearman_return_t gearman_wait(gearman_state_st *gearman);

/**
 * Get next connection that is ready for I/O.
 *
 * @param[in] gearman Structure previously initialized with gearman_create() or
 *  gearman_clone().
 * @return Connection that is ready for I/O, or NULL if there are none.
 */
GEARMAN_API
gearman_connection_st *gearman_ready(gearman_state_st *gearman);

/**
 * Test echo with all connections.
 *
 * @param[in] gearman Structure previously initialized with gearman_create() or
 *  gearman_clone().
 * @param[in] workload Data to send in echo packet.
 * @param[in] workload_size Size of workload.
 * @return Standard gearman return value.
 */
GEARMAN_API
gearman_return_t gearman_echo(gearman_state_st *gearman, const void *workload,
                              size_t workload_size);

/**
 * Free all packets for a gearman structure.
 *
 * @param[in] gearman Structure previously initialized with gearman_create() or
 *  gearman_clone().
 */
GEARMAN_API
void gearman_free_all_packets(gearman_state_st *gearman);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __GEARMAN_STATE_H__ */