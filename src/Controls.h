#ifndef __CONTROLS__
#define __CONTROLS__

typedef void (*KeyControlActionFn)( const char* name, bool pressed );
typedef void (*AxisControlActionFn)( const char* name, float absolute, float delta );

bool InitControls();
void DestroyControls();
void UpdateControls( float timeFrame );

/**
 * Key controls can either be pressed or released.
 *
 * @param name
 *      A unique identifier for this control.
 *
 * @param callback
 *      Optional function pointer that is called when the keys state changes.
 *
 * @param value
 *      Optional value pointer that always holds the state of the key.
 *
 * @return
 *      `false` if there is already a control with the same name.
 */
bool RegisterKeyControl( const char* name, KeyControlActionFn callback, bool* value );

/**
 * Axis controls have an value range from -1 to +1.
 *
 * @param name
 *      A unique identifier for this control.
 *
 * @param callback
 *      Optional function pointer that is called when the axis state changes.
 *
 * @param value
 *      Optional value pointer that always holds the value of the axis.
 *
 * @return
 *      `false` if there is already a control with the same name.
 */
bool RegisterAxisControl( const char* name, AxisControlActionFn callback, float* value );

// Internal: (needed by the binding implementations)
void HandleKeyEvent( int keyControlIndex, bool pressed );
void HandleAxisEvent( int axisControlIndex, float value );

#endif
