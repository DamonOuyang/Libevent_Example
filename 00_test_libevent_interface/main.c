#include <event2/event-config.h>
#include <stdio.h>
#include <event2/util.h>
#include <event2/buffer.h>
#include <event2/event.h>

void test_event_get_supported_methods()
{
    int i = 0;
    const char ** methods = event_get_supported_methods();
    printf("Starting Libevent %s. Available methods are:\n",event_get_version());
    for(i = 0;methods[i] != NULL; ++i)
    {
        printf("    %s\n",methods[i]);
    }
}

void test_event_method_new()
{
    struct event_base * base;
    enum event_method_feature f;
    base = event_base_new(); // 默认单个优先级
    event_base_priority_init(base,100); // 多个优先级0最高 到 100-1最低
    if(!base){
        puts("Couldn't get an event_base!");
    } else {
        printf("Using Libevent with backend metchod %s.",event_base_get_method(base));
    }
    f = event_base_get_features(base);
    if ((f & EV_FEATURE_ET))
        printf("  Edge-triggered events are supported.");
    if ((f & EV_FEATURE_O1))
        printf("  O(1) event notification is supported.");
    if ((f & EV_FEATURE_FDS))
        printf("  All FD types are supported.");
    puts("");
    event_base_free(base);
}

void test_event_reinit(struct event_base *ev_base)
{
    struct event_base * base = event_base_new();
    /*... add some events to the event_base ...*/
    if(fork()){
        /*In parent*/
        //continue_running_child(base);/*...*/
    }else{
        /*in child*/
        event_reinit(base);
        //continue_running_child(base);/*...*/
    }
}

/* Here's a callback function that calls loopbreak */
void cb(int sock,short what,void * arg)
{
    struct event_base * base = arg;
    event_base_loopbreak(base);
}

void main_loop(struct event_base* base,evutil_socket_t watchdog_fd)
{
    struct event *watchdog_event;
    /* Construct a new event to trigger whenever there are any bytes to
       read from a watchdog socket.  When that happens, we'll call the
       cb function, which will make the loop exit immediately without
       running any other active events at all.
     */
    watchdog_event = event_new(base,watchdog_fd,EV_READ,cb,base);

    event_add(watchdog_event,NULL);

    event_base_dispatch(base);
}

void run_base_with_ticks(struct event_base *base)
{
    struct timeval ten_sec;

    ten_sec.tv_sec = 10;
    ten_sec.tv_usec = 0;
    /* Now we run the event_base for a series of 10-second intervals, printing
       "Tick" after each.  For a much better way to implement a 10-second
       timer, see the section below about persistent timer events. */
    while(1){
        /* This schedules an exit ten seconds from now.*/
        event_base_loopexit(base,&ten_sec);

        event_base_dispatch(base);
        puts("Tick");
    }
}

/*
#define EV_TIMEOUT      0x01
#define EV_READ         0x02
#define EV_WRITE        0x04
#define EV_SIGNAL       0x08
#define EV_PERSIST      0x10
#define EV_ET           0x20
typedef void (*event_callback_fn)(evutil_socket_t, short, void *);
struct event *event_new(struct event_base *base, evutil_socket_t fd,
    short what, event_callback_fn cb,
    void *arg);
void event_free(struct event *event);
 */

void cb_func(evutil_socket_t fd,short what,void* arg)
{
    const char *data = arg;
    printf("Got an event on socket %d:%s%s%s%s [%s]",
         (int) fd,
         (what&EV_TIMEOUT) ? " timeout" : "",
         (what&EV_READ)    ? " read" : "",
         (what&EV_WRITE)   ? " write" : "",
         (what&EV_SIGNAL)  ? " signal" : "",
         data);
}

void main_loop2(evutil_socket_t fd1, evutil_socket_t fd2)
{
        struct event *ev1, *ev2;
        struct timeval five_seconds = {5,0};
        struct event_base *base = event_base_new();
        /* The caller has already set up fd1, fd2 somehow, and make them
           nonblocking. */
        ev1 = event_new(base, fd1, EV_TIMEOUT|EV_READ|EV_PERSIST, cb_func,
           (char*)"Reading event");
        ev2 = event_new(base, fd2, EV_WRITE|EV_PERSIST, cb_func,
           (char*)"Writing event");
        event_add(ev1, &five_seconds);
        event_add(ev2, NULL);
        event_base_dispatch(base);
}

#include <event2/event.h>
void read_cb(evutil_socket_t, short, void *);
void write_cb(evutil_socket_t, short, void *);
void main_loop(evutil_socket_t fd)
{
  struct event *important, *unimportant;
  struct event_base *base;
  base = event_base_new();
  event_base_priority_init(base, 2);
  /* Now base has priority 0, and priority 1 */
  important = event_new(base, fd, EV_WRITE|EV_PERSIST, write_cb, NULL);
  unimportant = event_new(base, fd, EV_READ|EV_PERSIST, read_cb, NULL);
  event_priority_set(important, 0);
  event_priority_set(unimportant, 1);
  /* Now, whenever the fd is ready for writing, the write callback will
     happen before the read callback.  The read callback won't happen at
     all until the write callback is no longer active. */
}

int main()
{
    //    struct event_base * base;
    //    enum event_method_feature f;
    //    base = event_base_new(); // 默认单个优先级
    //    run_base_with_ticks(base);
    //    printf("Hello World!\n");
    //    test_event_method_new();

    return 0;
}
