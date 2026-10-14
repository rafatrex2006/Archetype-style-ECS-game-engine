typedef enum {
    UI_EVENT_BUTTON_CLICKED,
} UI_EventType;

typedef struct {
    UI_EventType type;
    void *target;
} UI_Event;