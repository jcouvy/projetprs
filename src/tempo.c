#define _XOPEN_SOURCE

#include <SDL.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>

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
    unsigned long daytime;
    struct itimerval delay;
    void* event_param;
    struct event_s *prev;
    struct event_s *next;
} Event;

static Event* head = NULL;
static pthread_mutex_t mutex;

/* Adds an event to a double-linked list. The address of the list
is given in the argument head, the new element is a pointer towards
a pointer of an Event struct */
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


unsigned long second_to_micro(unsigned long sec){
    return sec * 1000000UL;
}

/* Returns the delay of an Event in micro seconds*/
unsigned long delay_of_event(Event *e)
{
    unsigned long delay_sec_e, delay_micro_e, delay_e;

    delay_sec_e       =   e->delay.it_value.tv_sec;
    delay_micro_e     =   e->delay.it_value.tv_usec;
    delay_e           =   second_to_micro(delay_sec_e) + delay_micro_e;

    return e->daytime + delay_e;
}

/* Returns True if the Event A happens before Event B */
bool compare_delay(Event* a, Event* b)
{
   unsigned long delay_a =   delay_of_event(a);
   unsigned long delay_b =   delay_of_event(b);

   return delay_a < delay_b;
}

/* Swaps the data between two Events parsed in parameters,
previous and next elements are left intact */
void swap(Event* a, Event* b)
{
    struct itimerval tmp_delay;
    void* tmp_param;

    tmp_delay = a->delay;
    tmp_param = a->event_param;
    a->delay = b->delay;
    a->event_param = b->event_param;
    b->delay = tmp_delay;
    b->event_param = tmp_param;
}

/* Bubblesort function to sort a linked list of Events. The sorting
is done by the ascending delay time */
void sort_events(Event** head)
{
    bool swapped;
    Event* curr;
    Event* last = NULL;

    /* Checking for empty list */
    if (*head == NULL)
        return;
    do
    {
        swapped = false;
        curr = *head;

        while (curr->next != last)
        {
            /* Compares if the current Event happens sooner than the next */
            if (!compare_delay(curr, curr->next))
            {
                swap(curr, curr->next);
                swapped = 1;
            }
            curr = curr->next;
        }
        last = curr;
    }
    while (swapped);
}

/* Prints the content of the linked list of Events */
void print_events(Event* head)
{
    Event *tmp = head;
    printf("Queued Events:\n");
    while (tmp->next != NULL)
    {
        printf("[Event: %lus %luµs] -> ", tmp->delay.it_value.tv_sec, tmp->delay.it_value.tv_usec);
        tmp = tmp->next;
    }
    printf("[Event: %lus %luµs]\n", tmp->delay.it_value.tv_sec, tmp->delay.it_value.tv_usec);

}

void signal_handler(int signo)
{
    if (signo == SIGALRM)
    {
        // pthread_mutex_lock(&mutex);
        sdl_push_event(head->event_param);
        printf("\nThread: %p Pid: %d\n", (void*)pthread_self(), getpid());
        // print_events(head);
        Event* tmp = head;
        head = head->next;
        free(tmp);
        if (head != NULL)
        {
            unsigned long timeForNext = head->daytime + head->delay.it_value.tv_sec * 1000000UL + head->delay.it_value.tv_usec;
            unsigned long timeBeforeNext = timeForNext - get_time();
            printf("New SIGALRM in: %lums\n", timeBeforeNext / 1000);
            struct itimerval nextTime;
            nextTime.it_value.tv_sec = timeBeforeNext / 1000000UL;
            nextTime.it_value.tv_usec = timeBeforeNext % 1000000UL;
            setitimer(ITIMER_REAL, &(nextTime), NULL);
        }
        // pthread_mutex_unlock(&mutex);
    }
}

void* daemon_handler(void* argp)
{
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGALRM);
    sigaction(SIGALRM, &sa, NULL);

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGALRM);

    while (1)
    {
        sigsuspend(&mask);
    }
}

// timer_init returns 1 if timers are fully implemented, 0 otherwise
int timer_init (void)
{

    pthread_t daemon;
    if (pthread_create(&daemon, NULL, &daemon_handler, NULL) != 0)
    {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    pthread_mutex_init(&mutex, NULL);

    // timer_set(1200, NULL);
    // timer_set(2500, NULL);
    // timer_set(1500, NULL);
    return 1; // Implementation not ready
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
    e->daytime = get_time();
    e->event_param = param;

    add_event(&head, &e);
    pthread_mutex_lock(&mutex);
    sort_events(&head);
    // The shortest delay is the head after each call of timer_set().
    if (head == e)
        setitimer(ITIMER_REAL, &head->delay, NULL);
    pthread_mutex_unlock(&mutex);
}

#endif
