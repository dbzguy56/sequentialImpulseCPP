# Sequential Impulse

## About the project
This project is a work in progress aimed at simulating collision detection and response through sequential impulse in 3D. C++, OpenGL are used in this project.

Currently the code detects collision using the [gjk algorithm](https://en.wikipedia.org/wiki/Gilbert%E2%80%93Johnson%E2%80%93Keerthi_distance_algorithm). Another great interactive demo of the gjk (and minkowski difference) is located at [this page](https://cse442-17f.github.io/Gilbert-Johnson-Keerthi-Distance-Algorithm/).

Originally I started this project in Jai, check it out at this [link](https://github.com/dbzguy56/sequentialImpulse).

## Video on Youtube
<a href="http://www.youtube.com/watch?feature=player_embedded&v=eXs0cLVw3w8" target="_blank">
 <img src="http://img.youtube.com/vi/eXs0cLVw3w8/mqdefault.jpg" alt="Watch the video" width="240" height="180" border="10" />
</a>


## Images
![gjk_1](/assets/images/gjk_1.png)
![gjk_2](/assets/images/gjk_2.png)
![gjk_3](/assets/images/gjk_3.png)

From the images above you can see that when the red tetrahedron collides with the blue cube, the green simplex appears. The gjk algorithmn relies on a support function that generates the points that would lie on the minkowski difference of the two colliding objects. These points make up the green simplex, and if this simplex contains the origin, then that means the two objects are colliding! (The black cube being the origin in this case)


## Running the application
You can run the demo by running the following command at the base of the project:

```bash
.\run.bat
```


### Controls
| Input | Action |
| ----- | ------ |
| Mouse | Aim Camera |
| W/A/S/D | Camera Movement |
| Left/Right/Up/Down Arrow Keys | Move the tetrahedron around |
| Backspace | Exit program |

(Backspace because of my keyboard :D)


## Building

### Requirements
1. Visual Studio along with the `Desktop development with C++` module
2. You may need to edit the vcvarsall path in `vcvars.bat` file.
   - It might already set to the correct path but if not, you will need to find where your `vcvars64.bat` is located.
   - If it's not under the current set directory, it maybe under `C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat`


After that is setup, you build with simply by running the command.

```bash
.\build.bat
```
