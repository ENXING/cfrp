#ifndef __SESSION_H__
#define __SESSION_H__

struct session
{
    char *session_id;
    void *ptr;
};

extern struct session *make_session();

extern int set_attr(const char *attr_name, const void *attr_value);

extern void *get_attr(const char *attr_name);

#endif