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

typedef struct event_s {
    struct itimerval delay;
    void* event_param;
    struct event_s *next;
} Event;

Event* head = NULL;

#ifdef PADAWAN

void add_event(Event* head, Event* new, unsigned long int sec, unsigned long int usec, void* param)
{
    Event* curr = malloc(sizeof(Event));
    curr = head;

    unsigned long int delay = sec + usec * 1000000;
    if (head == NULL)
    {
        head = new;
        head->next = NULL;
    }
    else if (delay < head->delay.it_value.tv_sec + head->delay.it_value.tv_usec * 1000000)
    {
        new->next = head;
        head = new;
    }
    else
    {
        while (curr->next != NULL)
        {
            if (delay < curr->next->delay.it_value.tv_sec + curr->next->delay.it_value.tv_usec * 1000000)
            {
                new->next = curr->next;
                curr->next = new;
                break;
            }
            curr = curr->next;
        }
        if (curr->next == NULL)
        {
            curr->next = new;
        }
    }

    curr->delay.it_value.tv_sec  = sec;
    curr->delay.it_value.tv_usec = usec;
    curr->event_param            = param;
}


void print_events(Event* head)
{
    Event* curr = malloc(sizeof(Event));
    curr = head;

    if (head != NULL)
    {
        while (curr->next != NULL)
        {
            printf("delay: %lu sec %lu usec\n", curr->delay.it_value.tv_sec, curr->delay.it_value.tv_usec);
            curr = curr->next;
        }
    }
}

void handler(int signo)
{
    if (signo == SIGALRM) {
        printf("Hello je suis le thread %p\n", (void*)pthread_self());
        printf ("sdl_push_event(%p) appelée au temps %ld\n", NULL, get_time());
    }
}

void* deamon_handler(void* argp)
{

    printf("JE SUIS LE DEMON %d\n", getpid());

    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGALRM);

    timer_set(5500, NULL);
    timer_set(1000, NULL);
    print_events(head);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    while (1)
    {
        sigsuspend(&mask);
    }
}

// timer_init returns 1 if timers are fully implemented, 0 otherwise
int timer_init (void)
{
    head = malloc(sizeof(Event));
    pthread_t deamon;
    pthread_create(&deamon, NULL, &deamon_handler, NULL);
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
    e->event_param = param;
    add_event(head, e, delay_sec, delay_usec, param);

    setitimer(ITIMER_REAL, &e->delay, NULL);
}

#endif
