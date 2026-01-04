This is a DEMO for an issue with FLTK + Wayland + Cairo

The code in this branch is a stripped-down variant of the
Yoshimi soft synth https://yoshimi.sourceforge.io/

The code base of Yoshimi is quite elaborate and has a long history,
dating back into the late 90ies. All that time, the open source
contributors managed to keep it running and evolving.


For this PROBLEM DEMO version
 - the entire Synth Engine was removed
 - the CMake build is also reduced to the bare minimum
 - it runs single threaded, with the UI in the main thread
 - the UI only has some minimal widgets (and looks strange,
   since also the theming support was removed)
 - in this branch there is a ton of unused code, and a lot
   of undefined symbols -- it just passes the compiler.

Build:

 - on Debian `apt install build-essential cmake libfltk1.4-dev`
 - `mkdir build`
 - `cmake -S src -B build`
 - `cmake --build build -- -j8`

then launch `build/yoshimi` and turn the "volume" dial

Observations:

 - When running under X11 (with FLTK 1.4) :
 
   * pop-up is positioned as intended, close to the rotary knob
   * BUT Cairo drawing not visible (no red line)
    
 - When running under Wayland
    
   * pop-up appears in the middle of the window
   * BUT Cairo drawing works: there is a dynamic red line


