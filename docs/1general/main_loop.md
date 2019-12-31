## The main loop and GLUT callbacks
The freeglut library contains many useful game development tools, including a game loop with a callback
system, which allows registering various functions that get executed when a certain event happens. The
callback used in this project include input callbacks, timed callbacks and window resize callbacks.

#### glutTimerFunc
Probably the most important callback type is the timed callback function. It allows registering functions that will be executed
after a certain time has passed ([read more](https://www.opengl.org/resources/libraries/glut/spec3/node64.html)).

The most obvious usage is to have the callback run a scheduled task or to have a function executed every n miliseconds.

```c
void render_timer_callback(int value) {

	glutPostRedisplay();
	glutTimerFunc(16, render_timer_callback, value);
}
```
The above example from [renderer.c](../../rogal/source/render/renderer.c) once executed will be executed every 
16 miliseconds, because after running the glutPostRedisplay() function it registers itself to run again after that period of time.

#### First callback registration
The program's entry point is located in the [main.c](../../rogal/source/main.c) file. The most important
callbacks are registered after the window is created (described in detail [here]()).

The function **void register_glut_callbacks(void)** is responsible for registering the following callbacks:
- glutDisplayFunc - called when GLUT is ordered to refresh the game screen
- render_timer_callback - registers a timed event that will execute 60 times per second to tell GLUT that the screen should be refreshed
- glutReshapeFunc - called when the game window is resized
- logic loop - another timed callback which will update some of the game components (texure animations and particles)
- glutKeyboardFunc - main keyboard input callback. Called when a character key is pressed
- glutSpecialFunc - keyboard input callback for the special keys (eg. arrow keys)
- glutMouseFunc - mouse click callback function

#### Finishing the game initialization
After all callbacks are registered and all other game modules are initialized the control is given over to the GLUT main loop using 
**glutMainLoop()** function. After that only the callbacks are used to control the program's state.

- Next article: [Vectors](vectors.md)