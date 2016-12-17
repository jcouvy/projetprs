#define _XOPEN_SOURCE

#include <SDL.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>

#include "timer.h"

// Return number of elapsed µsec since... a long time ago
static unsigned long get_time (void)
{
  struct timeval tv;

  gettimeofday (&tv ,NULL);

  // Only count seconds since beginning of 2016 (not jan 1st, 1970)
  tv.tv_sec -= 3600UL * 24 * 365 * 46;

  return tv.tv_sec * 1000000UL + tv.tv_usec;
}

#ifdef PADAWAN

typedef struct event_s {
    struct itimerval delay;
    void* event_param;
    struct event_s *prev;
    struct event_s *next;
} Event;

static Event* head = NULL;

void add_event(Event** head, Event** new_event)
{
    if (*head == NULL)
    {
        *head = *new_event;
        (*head)->prev = NULL;
        (*head)->next = NULL;
        return;
    }

    Event *tmp = *head;
    while (tmp->next != NULL)
    {
        tmp = tmp->next;
    }

    (*new_event)->prev = tmp;
    (*new_event)->next = NULL;
    tmp->next = *new_event;
}

bool compare_delay(Event** a, Event** b)
{
    //TODO...
    //Returns True if Event a is happening before Event b
}

void sort_event(Event** head)
{
    //TODO...
}

void print_events(Event* head)
{
    Event *tmp = head;
    printf("\nQueued Events:\n");
    while (tmp->next != NULL)
    {
        printf("[Event: %lu sec %lu usec] -> ", tmp->delay.it_value.tv_sec, tmp->delay.it_value.tv_usec);
        tmp = tmp->next;
    }
    printf("[Event: %lu sec %lu usec]\n\n", tmp->delay.it_value.tv_sec, tmp->delay.it_value.tv_usec);
}

void signal_handler(int signo)
{
    if (signo == SIGALRM) {
        printf ("sdl_push_event(%p) appelée au temps %ld\n", NULL, get_time());
    }
}

void* daemon_handler(void* argp)
{

    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGALRM);

    timer_set(5500, NULL);
    timer_set(1500, NULL);
    timer_set(500, NULL);
    timer_set(3500, NULL);
    print_events(head);

    while (1)
    {
        sigsuspend(&mask);
    }
}

// timer_init returns 1 if timers are fully implemented, 0 otherwise
int timer_init (void)
{

    pthread_t daemon;
    pthread_create(&daemon, NULL, &daemon_handler, NULL);

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    return 0; // Implementation not ready
}

void timer_set (Uint32 delay, void *param)
{
    unsigned long int delay_sec = delay / 1000;
    unsigned long int delay_usec = (delay % 1000) * 1000;
    printf("NEW TIMER second: %lu, microsec: %lu\n", delay_sec, delay_usec);

    Event* e = malloc(sizeof(Event));
    e->delay.it_value.tv_sec = delay_sec;
    e->delay.it_value.tv_usec = delay_usec;
    e->delay.it_interval.tv_sec = 0;
    e->delay.it_interval.tv_usec = 0;
    e->event_param = param;
    e->next = NULL;
    add_event(&head, &e);
    sort_event(&head);

    setitimer(ITIMER_REAL, &e->delay, NULL);
}

#endif
