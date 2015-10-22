#ifndef __HOOKS_H__
#define __HOOKS_H__

/*
Description:
Use this to do a pre-hook during setpinval, ie change the value of a pin
*/

int setpinval_hook(void *v, void *key, void *value);

#endif
