#ifndef __APOAPSIS_CONTROL_BINDINGS_JOYSTICK__
#define __APOAPSIS_CONTROL_BINDINGS_JOYSTICK__

bool InitJoystickBindings();
void DestroyJoystickBindings();
void UpdateJoystickControls( float timeFrame );
bool CreateJoystickBindingFromString( const char* str, int control );

#endif
