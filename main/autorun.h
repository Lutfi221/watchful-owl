#ifndef MAIN_AUTORUN
#define MAIN_AUTORUN

enum AutorunStatus
{
    AutorunDisabled,
    AutorunInvalid,
    AutorunEnabled
};

AutorunStatus getAutorunStatus();
void enableAutorun();
void disableAutorun();

#endif /* MAIN_AUTORUN */
