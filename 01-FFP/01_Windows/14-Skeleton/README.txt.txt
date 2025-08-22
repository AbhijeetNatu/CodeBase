Skeleton:-

small case for all functions as it needs to be used in all 6 technologies


iRetVal to catch Return Value

initialize() will return value which will be catched in iRetVal

Window Zorder Top and Focus on the window

ghwnd is for those outside winmain.. We have hwnd in winmain.. Hence use that inside winmain..
i.e for SetForegroundWindow(). We can use both.. But this is the logical reason.

Here, Window is brought to Foreground

SetFocus : WM_SETFOCUS will be sent to our window.

WM_ERASEBKGND : Window message (Erase Background)

resize : width(LOWORD(LPARAM)) and height (HIWORD(LPARAM))

WM_CLOSE

WM_DESTROY : 
1. unitialize
2. then Post Quit Message

height = 1


if gpFile valid, then call fclose
if window(ghwnd) valid, then Destroy Window and 


