#ifndef EVENT_H
#define EVENT_H

typedef enum EventType_e {
    KEY,
    MOUSE
} EventType;

typedef struct Event_s {
    EventType type;
} Event;

typedef struct KeyEvent_s {
    EventType type;
    int isUp;
    int code;
} KeyEvent;

typedef struct MouseEvent_s {
    EventType type;
    int x;
    int y;
    int buttons;
} MouseEvent;

#endif //EVENT_H