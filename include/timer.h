
#ifndef TIMER_IS_DEF
#define TIMER_IS_DEF

// Tyoe d'événements "utilisateur"
enum {
  TIMER_ONCE,
  TIMER_REPEAT
};

SDL_TimerID timer_set (Uint32 delay, int timer_type, void *param);

#endif
