#ifndef __SESSION_H__
#define __SESSION_H__

extern struct session *make_session();

extern int set_attr(const char *attr_name, const void *attr_value);

extern void *get_attr(const char *attr_name);

#endif